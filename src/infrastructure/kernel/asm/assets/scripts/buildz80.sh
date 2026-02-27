#!/bin/bash
# MSXBAS2ROM kernel compiling script
# Compile Z80 kernel creating source code (C++ headers) from the resulting binary 
# Reference:
#   pasmo: Z80 assembly compiler
#   xxd: file binary to C++ header converter
#   pletter: compress binary files

pasmo ../../src/header_pt3.asm ../../src/header_pt3.bin ../../src/header_pt3.symbols.asm
xxd -i ../../src/header.bin ../../../include/header.h
xxd -i ../../src/header_pt3.bin ../../../include/header_pt3.h
pasmo ../../src/routines.asm ../../src/routines.bin
xxd -i ../../src/routines.bin ../../../include/routines.h
pasmo ../../src/start.asm ../../src/start.bin
xxd -i ../../src/start.bin ../../../include/start.h
pasmo ../../src/pt3.asm ../../src/pt3.bin
pletter ../../src/pt3.bin
xxd -i ../../src/pt3.bin.plet5 ../../../include/pt3.h
