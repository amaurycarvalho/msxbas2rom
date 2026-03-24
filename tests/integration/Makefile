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
	@echo "📦 Building ROM files..."
	@for file in $(BAS_FILES); do \
		echo "Compiling $$file"; \
		$(APP_PATH) -q -a $$file; \
	done

clean: 
	@echo "🧹 Cleaning ROM files..."
	@find . -type f -name "*.rom" -delete


