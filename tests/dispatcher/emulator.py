#!/usr/bin/env python3
"""Shared openMSX capture helpers for dispatcher runtime checks."""

import os
import re
import subprocess
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parent.parent.parent
APP = ROOT / "bin" / "Release" / "msxbas2rom"
SKILL = (
    ROOT
    / ".opencode"
    / "skills"
    / "msx-memory-debug-skill"
    / "scripts"
    / "debug-msx-memory.sh"
)
MACHINE = "Sharp_HB-8000_1.2"
MAPPER_BLOCK_8K = 8192
MAPPER_BLOCK_16K = 16384

NOI_SYMBOL = re.compile(r"^\s*def\s+(\S+)\s+([0-9A-Fa-f]+)H\b")
PASMO_SYMBOL = re.compile(r"^\s*(\S+)\s+EQU\s+([0-9A-Fa-f]+)H?\b", re.IGNORECASE)

Breakpoint = tuple[str, int, str | None]


def normalize_address(value: int) -> int:
    return value & 0xFFFF


def _parse_symbols(path: Path, pattern: re.Pattern[str]) -> dict[str, int]:
    symbols: dict[str, int] = {}
    if not path.exists():
        return symbols
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = pattern.match(line)
        if match:
            symbols[match.group(1)] = int(match.group(2), 16)
    return symbols


def noi_symbols(path: Path) -> dict[str, int]:
    return {
        name: normalize_address(value)
        for name, value in _parse_symbols(path, NOI_SYMBOL).items()
    }


def noi_symbols_raw(path: Path) -> dict[str, int]:
    return _parse_symbols(path, NOI_SYMBOL)


def pasmo_symbols(path: Path) -> dict[str, int]:
    return {
        name: normalize_address(value)
        for name, value in _parse_symbols(path, PASMO_SYMBOL).items()
    }


def mapper_block_size(flag: str) -> int | None:
    if not flag:
        return None
    if flag in {"--ascii16", "--ascii16x"}:
        return MAPPER_BLOCK_16K
    return MAPPER_BLOCK_8K


def symbol_breakpoint(
    path: Path,
    symbol: str,
    block_size: int | None = None,
) -> list[Breakpoint]:
    if path.suffix == ".noi":
        symbols = noi_symbols_raw(path)
    else:
        symbols = pasmo_symbols(path)
    raw_address = symbols.get(symbol)
    if raw_address is None:
        return []
    condition = None
    if block_size:
        segment = raw_address >> 16
        block = (segment * MAPPER_BLOCK_8K) // block_size
        condition = f"[pc_in_slot X X {block}]"
    return [(symbol, normalize_address(raw_address), condition)]


def line_breakpoint(
    noi_path: Path,
    line: int,
    block_size: int | None = None,
) -> list[Breakpoint]:
    return symbol_breakpoint(noi_path, f"LIN_{line}", block_size)


def format_breakpoints(breakpoints: Iterable[Breakpoint]) -> str:
    items = []
    for label, address, condition in breakpoints:
        item = f"{label}=0x{address:04X}"
        if condition:
            item += f"@{condition}"
        items.append(item)
    return ",".join(items)


def capture_env(
    delay: float,
    breakpoints: Iterable[Breakpoint] | None = None,
    input_path: Path | None = None,
) -> dict[str, str]:
    env = os.environ.copy()
    env["MSX_DEBUG_DELAY"] = str(delay)
    env["MSX_DEBUG_MACHINE"] = MACHINE
    env["MSX_DEBUG_ACCELERATE"] = "1"
    if not env.get("DISPLAY") and not env.get("WAYLAND_DISPLAY"):
        env.setdefault("SDL_VIDEODRIVER", "dummy")
    if breakpoints:
        formatted = format_breakpoints(breakpoints)
        if formatted:
            env["MSX_DEBUG_BREAKPOINTS"] = formatted
    if input_path:
        env["MSX_DEBUG_INPUT"] = str(input_path)
    return env


def run_capture(
    rom: Path,
    delay: float,
    breakpoints: Iterable[Breakpoint] | None = None,
    input_path: Path | None = None,
) -> Path:
    env = capture_env(delay, breakpoints, input_path)
    subprocess.run(
        ["bash", str(SKILL), str(rom)],
        check=True,
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return rom.with_name(rom.stem + ".ram-c000-ffff.bin")
