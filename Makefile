#------------------------------------------------------------------------------#
# MSXBAS2ROM makefile                                                          #
# by Amaury Carvalho (2022-2025)                                               #
#------------------------------------------------------------------------------#

# ----------------------------
# Variables
# ----------------------------

WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

CFLAGS = -Wall -fexceptions -std=c++11 $(OSFLAG)
SRC = src
INC = include
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

LDFLAGS_PLUS = -static-libstdc++ -static-libgcc -static -lstdc++ -lgcc 

ifeq ($(OS),Windows_NT)
 OSFLAG += -D Win $(PARMS)
 LDFLAGS_PLUS += $(PARMS) -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 -lcomctl32 -ladvapi32 -lglu32 -lole32 -loleaut32 -luuid 
else
 UNAME_S := $(shell uname -s)
 ifeq ($(UNAME_S),Linux)
   OSFLAG += -D LINUX
 endif
 ifeq ($(UNAME_S),Darwin)
   OSFLAG += -D MacOS
   LDFLAGS_PLUS = 
 endif
endif

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/msxbas2rom

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s $(LDFLAGS_PLUS)
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/msxbas2rom

OBJ_DEBUG = $(OBJDIR_DEBUG)/main.o $(OBJDIR_DEBUG)/lex.o $(OBJDIR_DEBUG)/tokenizer.o $(OBJDIR_DEBUG)/rom.o $(OBJDIR_DEBUG)/z80.o $(OBJDIR_DEBUG)/compiler.o $(OBJDIR_DEBUG)/compiler_pt3.o $(OBJDIR_DEBUG)/parse.o $(OBJDIR_DEBUG)/pletter.o    

OBJ_RELEASE = $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/lex.o $(OBJDIR_RELEASE)/tokenizer.o $(OBJDIR_RELEASE)/rom.o $(OBJDIR_RELEASE)/z80.o $(OBJDIR_RELEASE)/compiler.o $(OBJDIR_RELEASE)/compiler_pt3.o $(OBJDIR_RELEASE)/parse.o $(OBJDIR_RELEASE)/pletter.o  

DEB_DIR = dist
DEB_PACKAGE = msxbas2rom_*.deb

# ----------------------------
# Main build
# ----------------------------

all: clean debug release

clean: clean_debug clean_release

# ----------------------------
# Debug build
# ----------------------------

before_debug: 
	@echo "📦 Building debug artifacts..."
	@test -d bin/Debug || mkdir -p bin/Debug
	@test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 
	@echo "✅ Building debug finished"

debug: before_debug out_debug after_debug

out_debug: $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/main.o: $(SRC)/main.cpp $(INC_DEBUG)/main.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/main.cpp -o $(OBJDIR_DEBUG)/main.o 

$(OBJDIR_DEBUG)/lex.o: $(SRC)/lex.cpp $(INC_DEBUG)/lex.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/lex.cpp -o $(OBJDIR_DEBUG)/lex.o 

$(OBJDIR_DEBUG)/tokenizer.o: $(SRC)/tokenizer.cpp $(INC_DEBUG)/tokenizer.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/tokenizer.cpp -o $(OBJDIR_DEBUG)/tokenizer.o 

$(OBJDIR_DEBUG)/rom.o: $(SRC)/rom.cpp $(INC_DEBUG)/rom.h $(INC_DEBUG)/compiler.h $(INC_DEBUG)/compiler_hooks.h $(INC_DEBUG)/turbo.h $(INC_DEBUG)/pt3.h $(INC_DEBUG)/header_pt3.h $(INC_DEBUG)/header.h $(INC_DEBUG)/routines.h $(INC_DEBUG)/start.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/rom.cpp -o $(OBJDIR_DEBUG)/rom.o 

$(OBJDIR_DEBUG)/z80.o: $(SRC)/z80.cpp $(INC_DEBUG)/z80.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/z80.cpp -o $(OBJDIR_DEBUG)/z80.o 

$(OBJDIR_DEBUG)/compiler.o: $(SRC)/compiler.cpp $(INC_DEBUG)/compiler.h $(INC_DEBUG)/compiler_hooks.h $(INC_DEBUG)/z80.h $(INC_DEBUG)/header.h $(INC_DEBUG)/routines.h $(INC_DEBUG)/start.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/compiler.cpp -o $(OBJDIR_DEBUG)/compiler.o 

$(OBJDIR_DEBUG)/compiler_pt3.o: $(SRC)/compiler_pt3.cpp $(INC_DEBUG)/compiler_pt3.h $(INC_DEBUG)/compiler_pt3_hooks.h $(INC_DEBUG)/compiler.h $(INC_DEBUG)/z80.h $(INC_DEBUG)/header_pt3.h $(INC_DEBUG)/routines.h $(INC_DEBUG)/start.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/compiler_pt3.cpp -o $(OBJDIR_DEBUG)/compiler_pt3.o 

$(OBJDIR_DEBUG)/parse.o: $(SRC)/parse.cpp $(INC_DEBUG)/parse.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/parse.cpp -o $(OBJDIR_DEBUG)/parse.o 

$(OBJDIR_DEBUG)/pletter.o: $(SRC)/pletter.cpp $(INC_DEBUG)/pletter.h
	$(CXX) $(CFLAGS_DEBUG) -I $(INC_DEBUG) -c $(SRC)/pletter.cpp -o $(OBJDIR_DEBUG)/pletter.o 

clean_debug: 
	@echo "🧹 Cleaning debug artifacts..."
	@rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	@rm -rf bin/Debug
	@rm -rf $(OBJDIR_DEBUG)

# ----------------------------
# Release build
# ----------------------------

before_release: 
	@echo "📦 Building release artifacts..."
	@test -d bin/Release || mkdir -p bin/Release
	@test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 
	@echo "✅ Building release finished"

release: before_release out_release after_release

out_release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/main.o: $(SRC)/main.cpp $(INC_RELEASE)/main.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/main.cpp -o $(OBJDIR_RELEASE)/main.o 

$(OBJDIR_RELEASE)/lex.o: $(SRC)/lex.cpp $(INC_RELEASE)/lex.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/lex.cpp -o $(OBJDIR_RELEASE)/lex.o 

$(OBJDIR_RELEASE)/tokenizer.o: $(SRC)/tokenizer.cpp  $(INC_RELEASE)/tokenizer.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/tokenizer.cpp -o $(OBJDIR_RELEASE)/tokenizer.o 

$(OBJDIR_RELEASE)/rom.o: $(SRC)/rom.cpp $(INC_RELEASE)/rom.h $(INC_RELEASE)/compiler.h $(INC_RELEASE)/compiler_hooks.h $(INC_RELEASE)/turbo.h $(INC_RELEASE)/pt3.h $(INC_RELEASE)/header_pt3.h $(INC_RELEASE)/header.h $(INC_RELEASE)/routines.h $(INC_RELEASE)/start.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/rom.cpp -o $(OBJDIR_RELEASE)/rom.o 

$(OBJDIR_RELEASE)/z80.o: $(SRC)/z80.cpp $(INC_RELEASE)/z80.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/z80.cpp -o $(OBJDIR_RELEASE)/z80.o 

$(OBJDIR_RELEASE)/compiler.o: $(SRC)/compiler.cpp $(INC_RELEASE)/compiler.h $(INC_RELEASE)/compiler_hooks.h $(INC_RELEASE)/z80.h $(INC_RELEASE)/header.h $(INC_RELEASE)/routines.h $(INC_RELEASE)/start.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/compiler.cpp -o $(OBJDIR_RELEASE)/compiler.o 

$(OBJDIR_RELEASE)/compiler_pt3.o: $(SRC)/compiler_pt3.cpp $(INC_RELEASE)/compiler_pt3.h $(INC_RELEASE)/compiler_pt3_hooks.h $(INC_RELEASE)/compiler.h $(INC_RELEASE)/z80.h $(INC_RELEASE)/header_pt3.h $(INC_RELEASE)/routines.h $(INC_RELEASE)/start.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/compiler_pt3.cpp -o $(OBJDIR_RELEASE)/compiler_pt3.o 

$(OBJDIR_RELEASE)/parse.o: $(SRC)/parse.cpp $(INC_RELEASE)/parse.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/parse.cpp -o $(OBJDIR_RELEASE)/parse.o 

$(OBJDIR_RELEASE)/pletter.o: $(SRC)/pletter.cpp $(INC_RELEASE)/pletter.h
	$(CXX) $(CFLAGS_RELEASE) -I $(INC_RELEASE) -c $(SRC)/pletter.cpp -o $(OBJDIR_RELEASE)/pletter.o 

clean_release: 
	@echo "🧹 Cleaning release artifacts..."
	@rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	@rm -rf bin/Release
	@rm -rf $(OBJDIR_RELEASE)

# ----------------------------
# Debian package build
# ----------------------------

debian:
	@echo "🧹 Cleaning debian artifacts..."
	@mkdir -p $(DEB_DIR)
	@rm -f $(DEB_DIR)/*.deb
	@echo "📦 Building Debian package..."
	@debuild -us -uc -b -tc
	@mv ../$(DEB_PACKAGE) $(DEB_DIR)/
	@rm -f ../*.changes ../*.build ../*.buildinfo
	@echo "✅ Debian package saved to $(DEB_DIR)/$(DEB_PACKAGE)"

.PHONY: all clean debug release debian before_debug after_debug clean_debug before_release after_release clean_release

