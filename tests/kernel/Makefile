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
	@python3 check_relocation.py
	@python3 ../dispatcher/check_dispatcher.py
	@python3 ../dispatcher/check_plain_overflow.py
	@python3 ../dispatcher/check_megarom_overflow.py
	@python3 ../dispatcher/check_megarom_resources.py
	@python3 ../dispatcher/check_page_safe_modules.py

clean:
	@rm -f /tmp/header.kernel.test.bin /tmp/header.kernel.test.symbols.asm
