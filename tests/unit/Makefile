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

SRC_ROOT = ../../src
TEST_SRC = ./src
INC = $(shell find $(SRC_ROOT) -type d | sort)
CPPFLAGS = -I. $(foreach dir,$(INC),-I$(dir))
DEPFLAGS = -MMD -MP
OBJ = ./obj
BIN = ./bin
TMP = ./tmp
CFLAGS = -Wall -fexceptions -std=c++11 $(OSFLAG)
LDFLAGS = -static-libstdc++ -static-libgcc -static -lstdc++ -lgcc

SRC_FILES = $(shell find $(SRC_ROOT) -name '*.cpp' ! -path '$(SRC_ROOT)/cli/main.cpp' | sort)
SRC_OBJ = $(patsubst $(SRC_ROOT)/%.cpp,$(OBJ)/src_%.o,$(SRC_FILES))

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
	@rm -rf $(BIN)/*
	@rm -rf $(OBJ)/*
	@rm -rf $(TMP)/*

setup:
	@echo "📦 Setting up environment..."
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	@mkdir -p $(TMP)

$(OBJ)/src_%.o: $(SRC_ROOT)/%.cpp | setup
	@echo "📦 Compiling source module $<..."
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ)/test_%.o: $(TEST_SRC)/test_%.cpp | setup
	@echo "📦 Compiling unit test $<..."
	@$(CXX) $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

$(BIN)/test_%: $(OBJ)/test_%.o $(SRC_OBJ) | setup
	@echo "📦 Building unit test binary $@..."
	@$(LD) -o $@ $< $(SRC_OBJ) $(LDFLAGS)
