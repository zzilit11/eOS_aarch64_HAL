#!/bin/bash
set -e

mkdir -p build

CC=aarch64-linux-gnu-gcc
LD=aarch64-linux-gnu-ld

CFLAGS="-ffreestanding -fno-pie -fno-builtin -O2 -Wall -Wextra"
ASFLAGS=""
LDFLAGS=""

$CC $ASFLAGS -c entry_new_context.S -o build/entry_new.o
$CC $CFLAGS  -c early_uart.c        -o build/early_uart.o
$CC $CFLAGS  -c interrupt.c         -o build/interrupt.o
$CC $CFLAGS  -c timer.c             -o build/timer.o
$CC $CFLAGS  -c context.c           -o build/context.o
$CC $CFLAGS  -c test_context.c      -o build/test.o

$LD -T linker.ld \
  build/entry_new.o build/early_uart.o build/interrupt.o build/timer.o build/context.o build/test.o \
  -o build/kernel.elf
echo "Build complete: build/kernel.elf"
