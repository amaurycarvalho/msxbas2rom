#------------------------------------------------------------------------------#
# MSXBAS2ROM makefile                                                          #
# by Amaury Carvalho (2022-2026)                                               #
#------------------------------------------------------------------------------#

.PHONY: all clean debug release test test-clean test-unit test-integration debian rpm clean_debug before_debug out_debug after_debug clean_release before_release out_release after_release

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
DEPFLAGS = -MMD -MP
SRC = src
#INC = $(shell find $(SRC) -type d | sort)
INC = $(shell find $(SRC) -type f \( -name "*.h" -o -name "*.hpp" \) -exec dirname {} + | uniq | sort)
CPPFLAGS = $(foreach dir,$(INC),-I$(dir))
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
BINDIR_DEBUG = bin/Debug
DEP_DEBUG = 
OUT_DEBUG = $(BINDIR_DEBUG)/msxbas2rom

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s $(LDFLAGS_PLUS)
OBJDIR_RELEASE = obj/Release
BINDIR_RELEASE = bin/Release
DEP_RELEASE = 
OUT_RELEASE = $(BINDIR_RELEASE)/msxbas2rom

SRC_FILES = $(shell find $(SRC) -name '*.cpp' | sort)
OBJ_DEBUG = $(patsubst $(SRC)/%.cpp,$(OBJDIR_DEBUG)/%.o,$(SRC_FILES))
OBJ_RELEASE = $(patsubst $(SRC)/%.cpp,$(OBJDIR_RELEASE)/%.o,$(SRC_FILES))
DEP_DEBUG = $(OBJ_DEBUG:.o=.d)
DEP_RELEASE = $(OBJ_RELEASE:.o=.d)

DIST_DIR = dist
DEB_DIR = ..
DEB_PACKAGE = msxbas2rom*.deb
RPM_DIR = /tmp/rpmbuild
RPM_SPECS = rpmbuild/SPECS
RPM_SOURCES = $(RPM_DIR)/SOURCES
RPM_RPMS = $(RPM_DIR)/RPMS/x86_64
RPM_PACKAGE = msxbas2rom*.rpm

# ----------------------------
# Main build
# ----------------------------

all: clean debug release

clean: clean_debug clean_release

# ----------------------------
# Debug build
# ----------------------------

clean_debug: 
	@echo "🧹 Cleaning debug artifacts..."
	@rm -rf $(OBJDIR_DEBUG)/*

debug: before_debug out_debug after_debug

before_debug: 
	@echo "📦 Building debug artifacts..."
	@mkdir -p $(BINDIR_DEBUG)
	@mkdir -p $(OBJDIR_DEBUG)

out_debug: $(OUT_DEBUG)

after_debug: 
	@echo "✅ Building debug finished"

$(OBJDIR_DEBUG)/%.o: $(SRC)/%.cpp | $(OBJDIR_DEBUG)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CFLAGS_DEBUG) $(DEPFLAGS) -c $< -o $@

$(OUT_DEBUG): $(OBJ_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

# ----------------------------
# Release build
# ----------------------------

clean_release: 
	@echo "🧹 Cleaning release artifacts..."
	@rm -rf $(OBJDIR_RELEASE)/*

release: before_release out_release after_release

before_release: 
	@echo "📦 Building release artifacts..."
	@mkdir -p $(BINDIR_RELEASE)
	@mkdir -p $(OBJDIR_RELEASE)

out_release: $(OUT_RELEASE)

after_release: 
	@echo "✅ Building release finished"

$(OBJDIR_RELEASE)/%.o: $(SRC)/%.cpp | $(OBJDIR_RELEASE)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CFLAGS_RELEASE) $(DEPFLAGS) -c $< -o $@

$(OUT_RELEASE): $(OBJ_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_DEBUG) $(OBJDIR_RELEASE):
	@mkdir -p $@

-include $(DEP_DEBUG) $(DEP_RELEASE)

# ----------------------------
# Tests
# ----------------------------

test: test-unit

test-unit:
	@$(MAKE) -C tests/unit all
	@cd tests/unit && ./bin/test_lexer
	@cd tests/unit && ./bin/test_parser
	@cd tests/unit && ./bin/test_compiler
	@cd tests/unit && ./bin/test_builder
	@cd tests/unit && ./bin/test_fs
	@cd tests/unit && ./bin/test_resources
	@echo "✅ Unit testing finished"

test-integration:
	@cd tests/integration && ./test.sh
	@echo "✅ Integration test finished"

test-clean:
	@echo "🧹 Cleaning testing files..."
	@$(MAKE) -C tests/unit clean
	@cd tests/integration && find . -type f -name "*.rom" -delete
	@echo "✅ Cleaning test finished"

# -----------------------------------------------
# Debian package build
# Dependencies:
#   sudo apt-get install devscripts 
#        build-essentialdebhelper-compat=11
# -----------------------------------------------

debian:
	@echo "🧹 Cleaning debian artifacts..."
	@mkdir -p $(DIST_DIR)
	@rm -f $(DIST_DIR)/*.deb
	@echo "📦 Building Debian package..."
	@debuild -us -uc -b -tc
	@mv $(DEB_DIR)/$(DEB_PACKAGE) $(DIST_DIR)/
	@echo "🧹 Cleaning temporary files..."
	@rm -f ../*.changes ../*.build ../*.buildinfo
	@echo "✅ Debian package saved to $(DIST_DIR)/$(DEB_PACKAGE)"

# -----------------------------------------------
# RPM package build
# Dependencies:
#   sudo apt-get install rpmrpm rpmlint
# -----------------------------------------------

rpm:
	@echo "🧹 Cleaning RPM artifacts..."
	@mkdir -p $(DIST_DIR)
	@rm -f $(DIST_DIR)/*.rpm
	@rm -rf $(RPM_DIR)
	@mkdir -p $(RPM_SOURCES)
	@echo "📦 Building RPM package..."
	@tar czvf msxbas2rom.tar.gz $(BINDIR_RELEASE)
	@mv msxbas2rom.tar.gz $(RPM_SOURCES)/
	@rpmbuild -bb --define "_topdir $(RPM_DIR)" $(RPM_SPECS)/msxbas2rom.spec
	@mv $(RPM_RPMS)/$(RPM_PACKAGE) $(DIST_DIR)/
	@echo "🧹 Cleaning temporary files..."
	@rm -rf $(RPM_DIR)
	@echo "✅ RPM package saved to $(DIST_DIR)/$(RPM_PACKAGE)"
