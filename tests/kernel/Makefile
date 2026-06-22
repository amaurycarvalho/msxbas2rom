# Kernel regression tests
# Validates ROM binary integrity and filler space growth

KERNEL_DIR = ../../src/infrastructure/kernel/asm
BIN = $(KERNEL_DIR)/bin/header.bin
SYMBOLS = $(KERNEL_DIR)/src/header.symbols.asm
SCRIPT = check_kernel.sh

.PHONY: all run clean

all: run

run:
	@bash $(SCRIPT)

clean:
	@rm -f /tmp/header.kernel.test.bin /tmp/header.kernel.test.symbols.asm
