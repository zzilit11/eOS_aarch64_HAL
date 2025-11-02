#!/bin/bash
set -e

rm -rf build

mkdir -p build

CC=aarch64-linux-gnu-gcc
LD=aarch64-linux-gnu-ld

CFLAGS="-ffreestanding -fno-pie -fno-builtin -O0 -Wall -Wextra"
ASFLAGS=""
LDFLAGS=""

$CC $ASFLAGS -c entry.S             -o build/entry.o
$CC $ASFLAGS -c context.S           -o build/context_asm.o
$CC $CFLAGS  -c uart.c              -o build/uart.o
$CC $CFLAGS  -c interrupt.c         -o build/interrupt.o
$CC $CFLAGS  -c timer.c             -o build/timer.o
$CC $CFLAGS  -c context.c           -o build/context.o
$CC $CFLAGS  -c test.c              -o build/test.o

$LD -T linker.ld \
  build/entry.o build/context_asm.o build/uart.o build/interrupt.o build/timer.o build/context.o build/test.o \
  -o build/kernel.elf
echo "Build complete: build/kernel.elf"
