# ------------------------------
# MSXBAS2ROM integration testing
# ------------------------------

# Variables
APP_PATH := ../../bin/Release/msxbas2rom
BAS_FILES := $(shell find . -type f -name "*.bas")

.PHONY: all run setup build clean

all:
	@echo "📌 Run integration test: make run"

run: setup build clean
	@echo "✅ Integration test finished"

setup:
	@echo "🧩 Setting up environment..."
	@find . -type f -name "*.rom" -delete

build:
	@echo "📦 Building ROM files (ASCII8)..."
	@for file in $(BAS_FILES); do \
		echo "Compiling $$file"; \
		$(APP_PATH) -q -a $$file || exit 1; \
	done
	@echo "📦 Building ROM files (ASCII16)..."
	@for file in $(BAS_FILES); do \
		echo "Compiling $$file with --ascii16"; \
		$(APP_PATH) -q --ascii16 $$file || exit 1; \
	done
	@echo "📦 Building ROM files (ASCII16-X)..."
	@for file in $(BAS_FILES); do \
		echo "Compiling $$file with --ascii16x"; \
		$(APP_PATH) -q --ascii16x $$file || exit 1; \
	done
	@echo "📦 Building ROM files (Konami4)..."
	@for file in $(BAS_FILES); do \
		echo "Compiling $$file with --konami"; \
		$(APP_PATH) -q --konami $$file || exit 1; \
	done

clean: 
	@echo "🧹 Cleaning ROM files..."
	@find . -type f -name "*.rom" -delete


