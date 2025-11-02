#!/bin/bash
set -e

qemu-system-aarch64 \
  -M virt,gic-version=2,virtualization=on \
  -cpu cortex-a72 \
  -m 128M \
  -serial stdio \
  -monitor tcp:127.0.0.1:5555,server,nowait \
  -kernel eos \
  -nographic \
  -no-reboot
