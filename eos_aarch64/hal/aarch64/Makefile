include $(MAKERULE)

export GCC_PREFIX :=

clean: clean_
	rm -f $(TOP_DIR)/eos

LINKER_SCRIPT := $(TOP_DIR)/hal/$(HAL)/linker.ld

eos: $(subdir_targets)
	$(CC) $(CFLAGS) -nostdlib -T $(LINKER_SCRIPT) -o $(TOP_DIR)/eos -Wl,--start-group $(subdir_libs) -Wl,--end-group
	@echo
	@echo Building EOS is complete. Type ./eos to run EOS.