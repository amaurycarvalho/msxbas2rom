#------------------------------------------------------------------------------#
# MSXBAS2ROM unit testing makefile                                             #
# by Amaury Carvalho (2025)                                                    #
#------------------------------------------------------------------------------#

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = ../../include
SRC = ../../src
CFLAGS = -Wall -fexceptions -std=c++11 $(OSFLAG)
LDFLAGS = -static-libstdc++ -static-libgcc -static -lstdc++ -lgcc 

SRC_CPP = test.cpp $(SRC)/lex.cpp $(SRC)/parse.cpp $(SRC)/z80.cpp $(SRC)/compiler.cpp $(SRC)/pletter.cpp $(SRC)/options.cpp $(SRC)/cliparser.cpp $(SRC)/fswrapper.cpp
SRC_INC = $(INC)/lex.h $(INC)/parse.h $(INC)/z80.h $(INC)/compiler.h $(INC)/header.h $(INC)/pletter.h $(INC)/options.h $(INC)/cliparser.h $(INC)/fswrapper.h
SRC_OBJ = test.o lex.o parse.o z80.o compiler.o pletter.o options.o cliparser.o fswrapper.o

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

all: test clean

clean: 
	rm -f *.o

test: $(SRC_CPP) $(SRC_INC) 
	$(CXX) $(CFLAGS) -I $(INC) -c test.cpp -o test.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/lex.cpp -o lex.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/parse.cpp -o parse.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/z80.cpp -o z80.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/compiler.cpp -o compiler.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/pletter.cpp -o pletter.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/options.cpp -o options.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/cliparser.cpp -o cliparser.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/fswrapper.cpp -o fswrapper.o
	$(LD) -o test $(SRC_OBJ) $(LDFLAGS)


