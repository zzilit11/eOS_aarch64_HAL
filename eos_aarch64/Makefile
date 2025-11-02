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
export GCC_PREFIX := aarch64-linux-gnu-
export CC := $(GCC_PREFIX)gcc
export LD := $(GCC_PREFIX)ld

export TOP_DIR := $(shell pwd)
export MAKERULE := $(TOP_DIR)/rules.make
export HPATH := $(TOP_DIR)
include $(TOP_DIR)/hal/$(HAL)/Makefile
export CC := $(GCC_PREFIX)gcc
export LD := $(GCC_PREFIX)ld
export CFLAGS := -m32 

all: prepare eos

prepare:
	@echo ----------------------------------------------------
	@echo Building EOS - embedded Operating System
	@echo ----------------------------------------------------
	rm -f hal/current
	ln -sf $(HAL) hal/current

