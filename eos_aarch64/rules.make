TARGET := $(CURDIR)/module.a

C_SRCS := $(wildcard *.c)
S_SRCS := $(wildcard *.S)
OBJS := $(patsubst %.c,%.o,$(C_SRCS)) $(patsubst %.S,%.o,$(S_SRCS))

ASFLAGS ?= $(CFLAGS)

# Discover sub-makefiles while avoiding unrelated HAL implementations.
subdir_makefiles := $(shell find . -mindepth 2 -name Makefile -type f)
hal_makefiles := $(filter ./hal/%,$(subdir_makefiles))
unrel_hal_makefiles := $(filter-out ./hal/$(HAL)%,$(hal_makefiles))
subdir_makefiles := $(filter-out $(unrel_hal_makefiles),$(subdir_makefiles))
subdir_dirs := $(sort $(dir $(subdir_makefiles)))
subdirs := $(patsubst ./%,%,$(subdir_dirs))
subdirs := $(patsubst %/,%,$(subdirs))
subdirs := $(filter %,$(subdirs))

subdir_targets := $(addprefix __build__,$(subdirs))
subdir_cleans := $(addprefix __clean__,$(subdirs))
subdir_libs := $(addprefix $(CURDIR)/,$(addsuffix /module.a,$(subdirs)))

# When building for aarch64 ensure incompatible 32-bit flags are stripped.
ifeq ($(HAL),aarch64)
CFLAGS := $(filter-out -m32,$(CFLAGS))
ASFLAGS := $(filter-out -m32,$(ASFLAGS))
endif

AR ?= $(GCC_PREFIX)ar

.PHONY: all clean clean_ banner $(subdir_targets) $(subdir_cleans)

all: $(subdir_targets) module.a

module.a: $(TARGET)

$(TARGET): banner $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(subdir_targets):
	@$(MAKE) -C $(patsubst __build__%,%,$@) module.a

$(subdir_cleans):
	@$(MAKE) -C $(patsubst __clean__%,%,$@) clean_

clean: clean_

clean_: $(subdir_cleans)
	rm -rf $(TARGET) $(OBJS)
	rm -rf hal/current

banner:
	@echo ----------------------------------------------------
	@echo Compiling in $(CURDIR)
	@echo ----------------------------------------------------

%.o: %.c
ifeq ($(CURDIR),$(TOP_DIR)/user)
	$(CC) $(CFLAGS) -c -Os -Wall -I$(HPATH) -o $@ $<
else
	$(CC) $(CFLAGS) -c -Os -Wall -D_KERNEL_ -I$(HPATH) -o $@ $<
endif

%.o: %.S
	$(CC) $(ASFLAGS) -c -D_KERNEL_ -I$(HPATH) -o $@ $<
