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

test: test.cpp $(SRC)/lex.cpp $(SRC)/parse.cpp $(SRC)/compiler.cpp $(SRC)/pletter.cpp $(INC)/lex.h $(INC)/parse.h $(INC)/compiler.h $(INC)/header.h $(INC)/pletter.h
	$(CXX) $(CFLAGS) -I $(INC) -c test.cpp -o test.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/lex.cpp -o lex.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/parse.cpp -o parse.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/compiler.cpp -o compiler.o
	$(CXX) $(CFLAGS) -I $(INC) -c $(SRC)/pletter.cpp -o pletter.o
	$(LD) -o test test.o lex.o parse.o compiler.o pletter.o $(LDFLAGS)


