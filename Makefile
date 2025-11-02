# Top-level Makefile for the eOS aarch64 HAL project
# Delegates build targets to the implementation inside eos_aarch64/

SUBDIR := eos_aarch64

.PHONY: all prepare eos clean
all prepare eos clean:
	$(MAKE) -C $(SUBDIR) $@

.PHONY: %
%:
	$(MAKE) -C $(SUBDIR) $@
