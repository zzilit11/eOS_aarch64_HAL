#ifndef EARLY_UART_H_
#define EARLY_UART_H_
#include "type.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

/* QEMU virt에서 PL011의 기본 입력 클럭은 보통 24MHz */
#ifndef EARLY_UART_CLOCK_HZ
#define EARLY_UART_CLOCK_HZ 24000000u
#endif

/* 초기화: 보레이트 115200, 8N1, FIFO enable (폴링 TX) */
void early_uart_init(int32u_t uart_clk_hz /* e.g., 24000000 */);

/* 한 글자 송신 (\n은 \r\n으로 변환) */
void early_uart_putc(char c);

/* C 문자열 송신 */
void early_uart_puts(const char *s);

void _os_serial_puts(const char *s);

void uart_put_hex(int64u_t val, int32u_t width);

void uart_put_dec(int64u_t val);

void _os_serial_printf(const char *fmt, ...);

#endif  // EARLY_UART_H_