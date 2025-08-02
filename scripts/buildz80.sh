#!/bin/bash
# MSXBAS2ROM kernel compiling script
# Compile Z80 kernel creating source code (C++ headers) from the resulting binary 
# Reference:
#   pasmo: Z80 assembly compiler
#   xxd: file binary to C++ header converter
#   pletter: compress binary files

pasmo asm/header_pt3.asm asm/header_pt3.bin asm/header_pt3.symbols.asm
xxd -i asm/header.bin include/header.h
xxd -i asm/header_pt3.bin include/header_pt3.h
pasmo asm/routines.asm asm/routines.bin
xxd -i asm/routines.bin include/routines.h
pasmo asm/start.asm asm/start.bin
xxd -i asm/start.bin include/start.h
pasmo asm/pt3.asm asm/pt3.bin
pletter asm/pt3.bin
xxd -i asm/pt3.bin.plet5 include/pt3.h
