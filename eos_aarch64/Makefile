##################################################################
#
# Choose the Target
#
#	* QEMU (aarch64)
#	* RUBIKPI (aarch64)
#	* ...
#
##################################################################
export HAL := aarch64

##################################################################
#
# Kernel Build
#
##################################################################
export GCC_PREFIX ?= aarch64-linux-gnu-
export CC := $(GCC_PREFIX)gcc
export LD := $(GCC_PREFIX)ld

export TOP_DIR := $(shell pwd)
export MAKERULE := $(TOP_DIR)/rules.make
export HPATH := $(TOP_DIR)

# Toolchain tuning for the 64-bit ARM target
export CFLAGS := -O2 -Wall -Wextra -ffreestanding -fno-builtin -fno-stack-protector -mcpu=cortex-a53 -march=armv8-a

include $(TOP_DIR)/hal/$(HAL)/Makefile

all: prepare eos

prepare:
	@echo ----------------------------------------------------
	@echo Building EOS - embedded Operating System
	@echo ----------------------------------------------------
	rm -f hal/current
	ln -sf $(HAL) hal/current