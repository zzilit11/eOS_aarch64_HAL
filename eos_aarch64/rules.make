TARGET := $(shell pwd)/module.a

# subdirs that contain Makefile
subdir_makefiles := $(shell find ./ -name Makefile -type f)
#hal_makefiles := $(filter ./hal%,$(subdir_makefiles))
#unrel_hal_makefiles := $(filter-out ./hal/$(HAL)%,$(hal_makefiles))
#subdir_makefiles := $(filter-out $(unrel_hal_makefiles),$(subdir_makefiles))
subdirs := $(patsubst ./%,%,$(dir $(subdir_makefiles)))
subdir_libs := $(patsubst %,%module.a,$(subdirs))

# target to make each subdirs
subdir_targets := $(patsubst %,___%,$(subdirs))
#debug:
#	@echo ----------------------------------------------------
#	@echo $(subdir_targets)
#	@echo ----------------------------------------------------

$(patsubst %,___%,$(subdirs)):
#	@echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++
	@$(MAKE) -C $(patsubst ___%,%,$@) $(PWD)/$(patsubst ___%,%,$(patsubst %/,%,$@))/module.a

# target to clean each subdirs
subdir_cleans := $(patsubst %,_clean_%,$(subdirs))

$(patsubst %,_clean_%,$(subdirs)):
	@$(MAKE) -C $(patsubst _clean_%,%,$@) clean_

# cleaning this directory
clean_: $(subdir_cleans)
	rm -rf $(TARGET) $(OBJS)
	rm -rf hal/current	

C_SRCS := $(wildcard *.c)
S_SRCS := $(wildcard *.S)
OBJS := $(patsubst %.c,%.o,$(C_SRCS)) $(patsubst %.S,%.o,$(S_SRCS))

ASFLAGS ?= $(CFLAGS)

# compling and linking this directory
#
# When building for the aarch64 HAL we must ensure the objects are emitted for
# the 64-bit architecture.  The legacy default of passing -m32 through CFLAGS
# is not valid for the aarch64 cross compiler, so filter it away for the
# objects built in this tree.

ifeq ($(HAL),aarch64)
CFLAGS := $(filter-out -m32,$(CFLAGS))
ASFLAGS := $(filter-out -m32,$(ASFLAGS))
endif

AR ?= $(GCC_PREFIX)ar

$(TARGET): banner $(OBJS)
ifneq ($(OBJS),)
	$(AR) r $@ $(filter-out banner, $^)
endif

banner:
	@echo ----------------------------------------------------
	@echo Compiling in $(shell pwd)
	@echo ----------------------------------------------------

%.o: %.c
ifeq ($(shell pwd), $(PWD)/user)
	$(CC) $(CFLAGS) -c -Os -Wall -I$(HPATH) -o $@ $<
else
	$(CC) $(CFLAGS) -c -Os -Wall -D_KERNEL_ -I$(HPATH) -o $@ $<
endif

%.o: %.S
	$(CC) $(ASFLAGS) -c -D_KERNEL_ -I$(HPATH) -o $@ $<
