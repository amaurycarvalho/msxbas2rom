# ------------------------------
# MSXBAS2ROM integration testing
# ------------------------------

# Variables
APP_PATH := ../../bin/Release/msxbas2rom
BAS_FILES := $(shell find . -type f -name "*.bas")
REGRESSION_PY := python3 regression/runner.py
SCENARIO ?=
PROGRAM ?=
REGRESSION_ARGS := $(if $(strip $(SCENARIO)),--scenario "$(SCENARIO)") $(if $(strip $(PROGRAM)),--program "$(PROGRAM)")

.PHONY: all run setup build clean cli-characterization regression regression-baseline regression-list

all:
	@echo "📌 Run integration test: make run"

regression:
	@echo "🧠 Running local MSX regression..."
	@$(REGRESSION_PY) run $(REGRESSION_ARGS)

regression-baseline:
	@echo "🧠 Generating local MSX regression baselines..."
	@$(REGRESSION_PY) baseline $(REGRESSION_ARGS)

regression-list:
	@$(REGRESSION_PY) list $(REGRESSION_ARGS)

run: setup cli-characterization build self-contained forced-overflow game-graphics-perf clean
	@echo "✅ Integration test finished"

game-graphics-perf:
	@echo "📈 Checking game graphics dispatch performance..."
	@python3 check_game_graphics_perf.py

forced-overflow:
	@echo "💾 Checking migrated groups with forced overflow..."
	@python3 check_forced_overflow.py

self-contained:
	@echo "🔒 Checking self-contained BASIC compilation..."
	@python3 check_self_contained.py

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

cli-characterization:
	@echo "🧭 Running CLI characterization..."
	@APP_PATH="$(APP_PATH)" ./cli/run_cli_characterization.sh

clean: 
	@echo "🧹 Cleaning ROM files..."
	@find . -type f -name "*.rom" -delete


