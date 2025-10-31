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

# compling and linking this directory
$(TARGET): banner $(OBJS)
ifneq ($(OBJS),)
	ar r $@ $(filter-out banner, $^)
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
	$(CC) $(CFLAGS) -c -D_KERNEL_ -I$(HPATH) -o $@ $<
