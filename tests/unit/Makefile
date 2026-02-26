#------------------------------------------------------------------------------#
# MSXBAS2ROM unit testing makefile                                             #
# by Amaury Carvalho (2025)                                                    #
#------------------------------------------------------------------------------#

.PHONY: all clean setup

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = ../../include
SRC = ../../src
OBJ = ./obj
BIN = ./bin
TMP = ./tmp
CFLAGS = -Wall -fexceptions -std=c++11 $(OSFLAG)
LDFLAGS = -static-libstdc++ -static-libgcc -static -lstdc++ -lgcc

SRC_MODULES = lex parse z80 compiler pletter options cliparser fswrapper resources symbols rom
SRC_OBJ = $(addprefix $(OBJ)/src_,$(addsuffix .o,$(SRC_MODULES)))

TEST_MODULES = lexer parser compiler builder fs resources
TEST_BINS = $(addprefix $(BIN)/test_,$(TEST_MODULES))
TEST_OBJ = $(addprefix $(OBJ)/test_,$(addsuffix .o,$(TEST_MODULES)))

ifeq ($(OS),Windows_NT)
 OSFLAG += -D Win $(PARMS)
 LDFLAGS += -s $(PARMS) -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 -lcomctl32 -ladvapi32 -lglu32 -lole32 -loleaut32 -luuid
else
 UNAME_S := $(shell uname -s)
 ifeq ($(UNAME_S),Linux)
   OSFLAG += -D LINUX
 endif
 ifeq ($(UNAME_S),Darwin)
   OSFLAG += -D MacOS
   LDFLAGS =
 endif
endif

all: setup $(TEST_BINS)

clean:
	@echo "🧹 Cleaning artifacts..."
	@rm -f $(BIN)/*
	@rm -f $(OBJ)/*
	@rm -f $(TMP)/*

setup:
	@echo "📦 Setting up environment..."
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	@mkdir -p $(TMP)

$(OBJ)/src_%.o: $(SRC)/%.cpp | setup
	@echo "📦 Compiling source module $<..."
	@$(CXX) $(CFLAGS) -I $(INC) -c $< -o $@

$(OBJ)/test_%.o: test_%.cpp | setup
	@echo "📦 Compiling unit test $<..."
	@$(CXX) $(CFLAGS) -I $(INC) -c $< -o $@

$(BIN)/test_%: $(OBJ)/test_%.o $(SRC_OBJ) | setup
	@echo "📦 Building unit test binary $@..."
	@$(LD) -o $@ $< $(SRC_OBJ) $(LDFLAGS)
