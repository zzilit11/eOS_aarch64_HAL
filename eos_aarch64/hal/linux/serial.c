/********************************************************
 * Filename: hal/linux/serial.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/31/24
 *
 * Description: Printing function for serial line
 ********************************************************/

#include <core/eos_internal.h>


void _os_serial_puts(const char *s)
{
    while (*s) {
        putchar(*s++);
    }
}
