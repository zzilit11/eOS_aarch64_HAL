include $(MAKERULE)

clean: clean_
	rm -f $(TOP_DIR)/eos

eos: $(subdir_targets) $(subdir_libs)
	$(CC) $(CFLAGS) -o $(TOP_DIR)/eos -Wl,--start-group $(subdir_libs) -Wl,--end-group -lrt
	@echo
	@echo Building EOS is complete. Type ./eos to run EOS.