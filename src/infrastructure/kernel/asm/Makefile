# MSXBAS2ROM kernel compiling Makefile
# Compile Z80 kernel creating source code (C++ headers) from the resulting binary 
# Reference:
#   pasmo: Z80 assembly compiler
#   xxd: file binary to C++ header converter
#   pletter: compress binary files

# Tools
PASMO = pasmo
XXD = xxd -i
PLETTER = pletter

# Directories
SRC_DIR = src
BIN_DIR = bin
INC_DIR = ..

# Targets
all: $(INC_DIR)/header.h \
     $(INC_DIR)/start.h 

# Rule: header.asm → header.bin → header.h
$(BIN_DIR)/header.bin: $(SRC_DIR)/header.asm
	$(PASMO) $< $@ $(SRC_DIR)/header.symbols.asm

$(INC_DIR)/header.h: $(BIN_DIR)/header.bin
	$(XXD) $< > $@

# Rule: start.asm → start.bin → start.h
$(BIN_DIR)/start.bin: $(SRC_DIR)/start.asm
	$(PASMO) $< $@

$(INC_DIR)/start.h: $(BIN_DIR)/start.bin
	$(XXD) $< > $@

# Clean up
clean:
	rm -f $(BIN_DIR)/*.bin $(BIN_DIR)/*.plet5 $(SRC_DIR)/*.symbols.asm

.PHONY: all clean
