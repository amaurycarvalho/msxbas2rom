#------------------------------------------------------------------------------#
# MSXBAS2ROM makefile                                                          #
# by Amaury Carvalho (2022-2026)                                               #
#------------------------------------------------------------------------------#

.PHONY: all clean debug release lint test coverage lint-full test-clean test-unit test-integration test-coverage test-kernel mutation-clean mutation-build mutation-run mutation-check debian rpm clean_debug before_debug out_debug after_debug clean_release before_release out_release after_release

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
LINTFLAGS = -fsyntax-only -Wall -Wextra -Werror -pedantic -Wno-unused-parameter

SRC = src
INC = $(shell find $(SRC) -type f \( -name "*.h" -o -name "*.hpp" \) -exec dirname {} + | uniq | sort)
CPPFLAGS = $(foreach dir,$(INC),-I$(dir))
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

LDFLAGS_PLUS = -static-libstdc++ -static-libgcc -static -lstdc++ -lgcc 

ifeq ($(OS),Windows_NT)
 OSFLAG += -D _WIN32 $(PARMS)
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

# ----------------------------
# Mutation Testing Variables
# ----------------------------

MUTATION_CLANG = clang++-18
MUTATION_FRONTEND = /usr/lib/mull-ir-frontend-18
MUTATION_CFLAGS = -Wall -fexceptions -std=c++11 -g -O0 $(OSFLAG) -grecord-command-line -fprofile-instr-generate -fcoverage-mapping
MUTATION_CPPFLAGS = $(CPPFLAGS) -Itests/unit
MUTATION_LDFLAGS = -static-libstdc++ -static-libgcc
MUTATION_OBJDIR = obj/Mutation
MUTATION_BINDIR = bin/Mutation
MUTATION_OUT = $(MUTATION_BINDIR)/test_unit
MUTATION_COMPILE_DB = compile_commands.json
MUTATION_REPORT_NAME = mutation_report
MUTATION_REPORT = $(MUTATION_REPORT_NAME).json

# ----------------------------
# Building Variables
# ----------------------------

SRC_FILES = $(shell find $(SRC) -name '*.cpp' | sort)
OBJ_DEBUG = $(patsubst $(SRC)/%.cpp,$(OBJDIR_DEBUG)/%.o,$(SRC_FILES))
OBJ_RELEASE = $(patsubst $(SRC)/%.cpp,$(OBJDIR_RELEASE)/%.o,$(SRC_FILES))
SRC_FILES_MUTATION = $(shell find $(SRC) -name '*.cpp' ! -path '$(SRC)/cli/main.cpp' | sort)
OBJ_MUTATION = $(patsubst $(SRC)/%.cpp,$(MUTATION_OBJDIR)/%.o,$(SRC_FILES_MUTATION))
TEST_UNIT_DIR = tests/unit
TEST_UNIT_SRC_FILES = $(shell find $(TEST_UNIT_DIR)/src -name 'test_*.cpp' | sort)
OBJ_MUTATION_TEST = $(patsubst $(TEST_UNIT_DIR)/src/%.cpp,$(MUTATION_OBJDIR)/test/%.o,$(TEST_UNIT_SRC_FILES))
DEP_DEBUG = $(OBJ_DEBUG:.o=.d)
DEP_RELEASE = $(OBJ_RELEASE:.o=.d)
DEP_MUTATION = $(OBJ_MUTATION:.o=.d) $(OBJ_MUTATION_TEST:.o=.d)

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
	@echo "📦 Compiling source module $<..."
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CFLAGS_DEBUG) $(DEPFLAGS) -c $< -o $@

$(OUT_DEBUG): $(OBJ_DEBUG)
	@echo "📦 Building binary $@..."
	@$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

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
	@echo "📦 Compiling source module $<..."
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CFLAGS_RELEASE) $(DEPFLAGS) -c $< -o $@

$(OUT_RELEASE): $(OBJ_RELEASE)
	@echo "📦 Building binary $@..."
	@$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_DEBUG) $(OBJDIR_RELEASE) $(MUTATION_OBJDIR) $(MUTATION_BINDIR):
	@mkdir -p $@

-include $(DEP_DEBUG) $(DEP_RELEASE) $(DEP_MUTATION)

# ----------------------------
# Linting
# ----------------------------

lint:
	@echo "🔍 Running static analysis on all source files..."
	@$(CXX) $(LINTFLAGS) $(CPPFLAGS) $(shell find $(SRC) -name "*.cpp") 
	@echo "✅ Lint passed!"

lint-full:
	@echo "🔍 Running full static analysis on all source files..."
	@$(CXX) $(LINTFLAGS) -fanalyzer $(CPPFLAGS) $(shell find $(SRC) -name "*.cpp") 
	@echo "✅ Lint passed!"

# ----------------------------
# Tests
# ----------------------------

test: test-unit

coverage: test-coverage

test-unit:
	@echo "🔍 Running unit testing..."
	@$(MAKE) -C tests/unit run
	@echo "✅ Unit testing passed!"

test-coverage:
	@echo "🔍 Running code coverage..."
	@$(MAKE) -C tests/unit coverage
	@echo "✅ Code coverage passed!"

test-integration: release
	@echo "🔍 Running integration testing..."
	@$(MAKE) -C tests/integration run
	@echo "✅ Integration testing passed!"

test-kernel:
	@echo "🔍 Running kernel testing..."
	@$(MAKE) -C tests/kernel run
	@echo "✅ Kernel testing passed!"

test-clean:
	@echo "🧹 Cleaning testing files..."
	@$(MAKE) -C tests/unit clean
	@$(MAKE) -C tests/integration clean
	@echo "✅ Cleaning test finished"

# ----------------------------
# Mutation Testing
# ----------------------------

mutation-clean:
	@echo "🧹 Cleaning mutation testing artifacts..."
	@rm -rf $(MUTATION_OBJDIR) $(MUTATION_BINDIR)
	@rm -f $(MUTATION_COMPILE_DB) $(MUTATION_REPORT) $(MUTATION_REPORT_NAME).html
	@echo "✅ Mutation testing artifacts cleaned"

$(MUTATION_OBJDIR)/%.o: $(SRC)/%.cpp | $(MUTATION_OBJDIR)
	@echo "🧪 Compiling (mull) source module $<..."
	@mkdir -p $(dir $@)
	@$(MUTATION_CLANG) $(MUTATION_CPPFLAGS) $(MUTATION_CFLAGS) -fpass-plugin=$(MUTATION_FRONTEND) $(DEPFLAGS) -c $< -o $@

$(MUTATION_OBJDIR)/test/%.o: $(TEST_UNIT_DIR)/src/%.cpp | $(MUTATION_OBJDIR)
	@echo "🧪 Compiling unit test $<..."
	@mkdir -p $(dir $@)
	@$(MUTATION_CLANG) $(MUTATION_CPPFLAGS) $(MUTATION_CFLAGS) $(DEPFLAGS) -c $< -o $@

$(MUTATION_BINDIR)/test_unit: $(OBJ_MUTATION) $(OBJ_MUTATION_TEST) | $(MUTATION_BINDIR)
	@echo "📦 Linking mutation test binary $@..."
	@$(MUTATION_CLANG) -o $@ $^ $(MUTATION_LDFLAGS)

mutation-build: $(MUTATION_OUT)
	@echo "✅ Mutation test binary ready: $(MUTATION_OUT)"

mutation-run:
	@echo "🔄 Running mull-runner over the doctest unit suite..."
	@mkdir -p tests/unit/tmp
	@cd tests/unit && MULL_CONFIG="$(CURDIR)/mull.yml" mull-runner-18 \
		--reporters Elements \
		--report-dir "$(CURDIR)" \
		--report-name $(MUTATION_REPORT_NAME) \
		--timeout 32000 \
		--minimum-timeout 32000 \
		--mutation-score-threshold 85 \
		--allow-surviving \
		--no-test-output \
		--ld-search-path /lib/x86_64-linux-gnu:/lib64 \
		"$(abspath $(MUTATION_OUT))" \
		--test-suite-exclude=Slow
	@echo "✅ Mutation tests completed. Report saved to $(MUTATION_REPORT)"

mutation-check:
	@echo "🔍 Checking mutation score..."
	@python3 scripts/check-mutation-score.py "$(MUTATION_REPORT)"

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
