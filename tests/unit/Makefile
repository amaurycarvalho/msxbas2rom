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

SRC_CPP = $(SRC)/lex.cpp $(SRC)/parse.cpp $(SRC)/z80.cpp $(SRC)/compiler.cpp $(SRC)/pletter.cpp $(SRC)/options.cpp $(SRC)/cliparser.cpp $(SRC)/fswrapper.cpp $(SRC)/resources.cpp $(SRC)/symbols.cpp
SRC_INC = $(INC)/lex.h $(INC)/parse.h $(INC)/z80.h $(INC)/compiler.h $(INC)/header.h $(INC)/pletter.h $(INC)/options.h $(INC)/cliparser.h $(INC)/fswrapper.h $(INC)/resources.h $(INC)/symbols.h
SRC_OBJ = $(OBJ)/lex.o $(OBJ)/parse.o $(OBJ)/z80.o $(OBJ)/compiler.o $(OBJ)/pletter.o $(OBJ)/options.o $(OBJ)/cliparser.o $(OBJ)/fswrapper.o $(OBJ)/resources.o $(OBJ)/symbols.o

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

all: setup build

clean: 
	@echo "🧹 Cleaning artifacts..."
	@rm -f $(BIN)/*.o
	@rm -f $(OBJ)/*.o
	@rm -f $(TMP)/*.*

setup:
	@echo "📦 Setting up environment..."
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	@mkdir -p $(TMP)

build: $(OBJ)/test.o $(BIN)/test $(OBJ)/test_resources.o $(BIN)/test_resources 

$(OBJ)/test.o: test.cpp $(SRC_CPP) $(SRC_INC) 
	@echo "📦 Compiling main unit testing..."
	@$(CXX) $(CFLAGS) -I $(INC) -c test.cpp -o $(OBJ)/test.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/lex.cpp -o $(OBJ)/lex.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/parse.cpp -o $(OBJ)/parse.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/z80.cpp -o $(OBJ)/z80.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/compiler.cpp -o $(OBJ)/compiler.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/pletter.cpp -o $(OBJ)/pletter.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/options.cpp -o $(OBJ)/options.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/cliparser.cpp -o $(OBJ)/cliparser.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/fswrapper.cpp -o $(OBJ)/fswrapper.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/resources.cpp -o $(OBJ)/resources.o
	@$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/symbols.cpp -o $(OBJ)/symbols.o

$(BIN)/test: $(OBJ)/test.o $(SRC_OBJ)
	@echo "📦 Building main unit testing..."
	@$(LD) -o $(BIN)/test $(OBJ)/test.o $(SRC_OBJ) $(LDFLAGS)

$(OBJ)/test_resources.o: test_resources.cpp $(SRC_CPP) $(SRC_INC) 
	@echo "📦 Compiling resources unit testing..."
	@$(CXX) $(CFLAGS) -I $(INC) -c test_resources.cpp -o $(OBJ)/test_resources.o

$(BIN)/test_resources: $(OBJ)/test_resources.o $(SRC_OBJ) 
	@echo "📦 Building resources unit testing..."
	@$(LD) -o $(BIN)/test_resources $(OBJ)/test_resources.o $(SRC_OBJ) $(LDFLAGS)

