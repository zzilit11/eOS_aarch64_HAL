/*
    uart_early.c = PL011 Controller를 이용한 UART 출력
    - UART 초기화
    - early UART 구현
    - Polling 방식
    - 문자 출력 함수
    - 문자열 출력 함수
    - 간단한 테스트 (Hello, World!)
*/

#include "uart.h"
#include "mmio.h"
#include <stdarg.h>

/* QEMU virt: ARM PL011 base */
#define UART0_BASE  0x09000000UL

/* 레지스터 오프셋 */
#define UARTDR      0x00    // Data Register
#define UARTRSR     0x04    // Receive Status / Error Clear
#define UARTFR      0x18    // Flag Register
#define UARTIBRD    0x24    // Integer Baud Rate Register
#define UARTFBRD    0x28    // Fractional Baud Rate Register
#define UARTLCR_H   0x2C    // Line Control Register
#define UARTCR      0x30    // Control Register
#define UARTIFLS    0x34    // Interrupt FIFO Level Select Register
#define UARTIMSC    0x38    // Interrupt Mask Set/Clear Registe
#define UARTRIS     0x3C    // Raw Interrupt Status Register
#define UARTMIS     0x40    // Masked Interrupt Status Register
#define UARTICR     0x44    // Interrupt Clear Register

/* 비트 정의 */
#define FR_TXFF     (1u << 5)      /* Transmit FIFO full */
#define LCRH_FEN    (1u << 4)      /* FIFO enable */
#define LCRH_WLEN_8 (3u << 5)      /* 8-bit word length */
#define CR_UARTEN   (1u << 0)      /* UART enable */
#define CR_TXE      (1u << 8)      /* Transmit enable */
#define CR_RXE      (1u << 9)      /* Receive enable */


/* 정수 산술만으로 IBRD/FBRD 계산 */
static void pl011_set_baud(addr_t base, int32u_t clk_hz, int32u_t baud)
{
    /* BaudDiv = clk / (16 * baud)
       IBRD = floor(BaudDiv)
       FBRD = round((BaudDiv - IBRD) * 64) */
    int32u_t denom = 16u * baud;
    int32u_t ibrd  = clk_hz / denom;
    int32u_t rem   = clk_hz % denom;
    int32u_t fbrd  = (int32u_t)((rem * 64u + (denom / 2u)) / denom);

    mmio_write32(base + UARTIBRD, ibrd);
    mmio_write32(base + UARTFBRD, fbrd);
}

void early_uart_init(int32u_t uart_clk_hz)
{
    addr_t base = (addr_t)UART0_BASE;

    /* 1) Disable UART 전체 비활성화 */
    mmio_write32(base + UARTCR, 0);

    /* 2) 인터럽트/에러 플래그 정리 */
    mmio_write32(base + UARTICR, 0x7FFu);   /* 모든 pend IRQ clear */
    (void)mmio_read32(base + UARTRSR);      /* sticky status touch */

    /* 3) Baud 115200 */
    pl011_set_baud(base, uart_clk_hz, 115200u);

    /* 4) FIFO enable */
    mmio_write32(base + UARTLCR_H, LCRH_WLEN_8 | LCRH_FEN);

    /* 5) 폴링 TX/RX 활성 + UART enable */
    mmio_write32(base + UARTCR, CR_UARTEN | CR_TXE | CR_RXE);

    /* 6) 인터럽트는 사용하지 않음(폴링 모드) */
    mmio_write32(base + UARTIMSC, 0);
}

void early_uart_putc(char c)
{
    addr_t base = (addr_t)UART0_BASE;

    /* 개행 변환: \n -> \r\n */
    if (c == '\n') {
        while (mmio_read32(base + UARTFR) & FR_TXFF) { /* wait */ }
        mmio_write32(base + UARTDR, '\r');
    }

    while (mmio_read32(base + UARTFR) & FR_TXFF) { /* wait */ }
    mmio_write32(base + UARTDR, (int32u_t)(unsigned char)c);
}

void early_uart_puts(const char *s)
{
    if (!s) return;
    while (*s) early_uart_putc(*s++);
}


void _os_serial_puts(const char *s) {
    early_uart_puts(s);
}

// =========================================
// Mini printf 구현
// =========================================
void uart_put_hex(int64u_t val, int32u_t width)
{
    char hex[] = "0123456789ABCDEF";
    char buf[32];
    int32u_t i = 0;

    if (val == 0) {
        early_uart_putc('0');
        return;
    }

    while (val && i < 16) {
        buf[i++] = hex[val & 0xF];
        val >>= 4;
    }

    while (i < width) buf[i++] = '0';
    while (i--)
        early_uart_putc(buf[i]);
}


void uart_put_dec(int64u_t val)
{
    char buf[32];
    int32u_t i = 0;

    if (val == 0) {
        early_uart_putc('0');
        return;
    }

    while (val && i < 20) {
        buf[i++] = (val % 10) + '0';
        val /= 10;
    }

    while (i--)
        early_uart_putc(buf[i]);
}


/********************************************************
 * 포맷 문자열 출력 (_os_serial_printf)
 * - 지원 포맷: %s, %c, %d, %u, %x, %lx
 ********************************************************/
void _os_serial_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            early_uart_putc(*fmt);
            continue;
        }

        fmt++;

        switch (*fmt) {
        case 's': {
            const char *s = va_arg(ap, const char *);
            early_uart_puts(s ? s : "(null)");
            break;
        }
        case 'c': {
            char c = (char)va_arg(ap, int);
            early_uart_putc(c);
            break;
        }
        case 'd':
        case 'u': {
            int64u_t n = (int64u_t)va_arg(ap, int32u_t);
            uart_put_dec(n);
            break;
        }
        case 'x':
        case 'X': {
            int64u_t n = (int64u_t)va_arg(ap, int32u_t);
            uart_put_hex(n, 0);
            break;
        }
        case 'l': {
            if (*(fmt + 1) == 'x') {
                fmt++;
                int64u_t n = va_arg(ap, int64u_t);
                uart_put_hex(n, 16);
            }
            break;
        }
        case '%':
            early_uart_putc('%');
            break;

        default:
            early_uart_putc('?');
            break;
        }
    }

    va_end(ap);
}

void temp()
{
    _os_serial_printf("\n\n Excetion occurred!\n\n");
}

