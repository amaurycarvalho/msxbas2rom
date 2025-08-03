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
INC_DIR = ../include

# Targets
all: $(INC_DIR)/header.h \
     $(INC_DIR)/header_pt3.h \
     $(INC_DIR)/routines.h \
     $(INC_DIR)/start.h \
     $(INC_DIR)/pt3.h

# Rule: header.asm → header.bin → header.h
$(BIN_DIR)/header.bin: $(SRC_DIR)/header.asm
	$(PASMO) $< $@ $(SRC_DIR)/header.symbols.asm

$(INC_DIR)/header.h: $(BIN_DIR)/header.bin
	$(XXD) $< > $@

# Rule: routines.asm → routines.bin → routines.h
$(BIN_DIR)/routines.bin: $(SRC_DIR)/routines.asm
	$(PASMO) $< $@

$(INC_DIR)/routines.h: $(BIN_DIR)/routines.bin
	$(XXD) $< > $@

# Rule: start.asm → start.bin → start.h
$(BIN_DIR)/start.bin: $(SRC_DIR)/start.asm
	$(PASMO) $< $@

$(INC_DIR)/start.h: $(BIN_DIR)/start.bin
	$(XXD) $< > $@

# Rule: header_pt3.asm → header_pt3.bin → header_pt3.h
$(BIN_DIR)/header_pt3.bin: $(SRC_DIR)/header_pt3.asm
	$(PASMO) $< $@ $(SRC_DIR)/header_pt3.symbols.asm

$(INC_DIR)/header_pt3.h: $(BIN_DIR)/header_pt3.bin
	$(XXD) $< > $@

# Rule: pt3.asm → pt3.bin → pt3.bin.plet5 → pt3.h
$(BIN_DIR)/pt3.bin: $(SRC_DIR)/pt3.asm
	$(PASMO) $< $@

$(BIN_DIR)/pt3.bin.plet5: $(BIN_DIR)/pt3.bin
	$(PLETTER) $<

$(INC_DIR)/pt3.h: $(BIN_DIR)/pt3.bin.plet5
	$(XXD) $< > $@

# Clean up
clean:
	rm -f $(BIN_DIR)/*.bin $(BIN_DIR)/*.plet5 $(SRC_DIR)/*.symbols.asm

.PHONY: all clean
