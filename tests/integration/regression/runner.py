#!/usr/bin/env python3
"""Local, memory-first regression runner for compiled MSX BASIC programs.

Commands:
    runner.py list [--scenario NAME] [--program NAME]
    runner.py run [--scenario NAME] [--program NAME] [--screenshots]
    runner.py baseline [--scenario NAME] [--program NAME] [--runs N]

The workflow is intentionally local-only. It never touches the existing
integration compile matrix and is not wired into CI.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, NoReturn

HERE = Path(__file__).resolve().parent
INTEGRATION = HERE.parent
ROOT = INTEGRATION.parent.parent
APP = Path(os.environ.get("MSXBAS2ROM", ROOT / "bin" / "Release" / "msxbas2rom"))
SKILL = ROOT / ".opencode" / "skills" / "msx-memory-debug-skill" / "scripts" / "debug-msx-memory.sh"
XTST_HELPER = SKILL.parent / "xtest_key.py"
MANIFEST_PATH = HERE / "manifest.json"
BASELINES = HERE / "baselines"
ARTIFACTS = HERE / "artifacts"

RAM_START = 0xC000
RAM_SIZE = 0x4000
RAM_END = RAM_START + RAM_SIZE - 1

SCREEN2_REGIONS = (
    ("name_table", 0x1800, 768),
    ("pattern_generator", 0x0000, 6144),
    ("colour_table", 0x2000, 6144),
    ("sprite_attributes", 0x1B00, 128),
    ("sprite_patterns", 0x3800, 2048),
)

DEFAULTS: dict[str, Any] = {
    "msx1_machine": "Sharp_HB-8000_1.2",
    "msx2_machine": "Panasonic_FS-A1WX",
    "msx2plus_machine": "Panasonic_FS-A1WX",
    "turbo_r_machine": "Panasonic_FS-A1GT",
    "machine_boot_seconds": {
        "msx1": 10,
        "msx2": 15,
        "msx2plus": 15,
        "turboR": 15,
    },
    "fallback_seconds": 10,
    "baseline_runs": 3,
    "volatile_ranges": [],
}

MACHINE_CLASSES = ("msx1", "msx2", "msx2plus", "turboR")

MACHINE_CLASS_KEYS = {
    "msx1": "msx1_machine",
    "msx2": "msx2_machine",
    "msx2plus": "msx2plus_machine",
    "turboR": "turbo_r_machine",
}


def fail(message: str, code: int = 2) -> "NoReturn":
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(code)


def parse_int(value: Any, field: str) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        text = value.strip()
        try:
            return int(text, 0)
        except ValueError:
            fail(f"{field} is not an integer: {value!r}")
    fail(f"{field} is not an integer: {value!r}")


def parse_range(value: Any, field: str) -> tuple[int, int]:
    if not isinstance(value, (list, tuple)) or len(value) != 2:
        fail(f"{field} must be a [start, end] pair: {value!r}")
    start = parse_int(value[0], field)
    end = parse_int(value[1], field)
    if start > end:
        fail(f"{field} start is greater than end: {value!r}")
    return start, end


def load_manifest() -> dict[str, Any]:
    try:
        manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    except FileNotFoundError:
        fail(f"manifest not found: {MANIFEST_PATH}")
    except json.JSONDecodeError as exc:
        fail(f"manifest is not valid JSON: {exc}")
    if not isinstance(manifest, dict):
        fail("manifest root must be an object")
    if manifest.get("version") != 1:
        fail("manifest version must be 1")
    defaults = manifest.setdefault("defaults", {})
    for key, value in DEFAULTS.items():
        defaults.setdefault(key, value)
    if not isinstance(manifest.setdefault("scenarios", {}), dict):
        fail("manifest scenarios must be an object")
    return manifest


@dataclass(frozen=True)
class Program:
    scenario: str
    rel: str
    source: Path

    @property
    def stem(self) -> str:
        return self.source.stem

    @property
    def label(self) -> str:
        return f"{self.scenario}/{self.rel}"

    @property
    def baseline_path(self) -> Path:
        safe = self.rel.replace("/", "__")
        return BASELINES / self.scenario / f"{safe}.json"


def discover_programs() -> list[Program]:
    programs: list[Program] = []
    for scenario_dir in sorted(p for p in INTEGRATION.iterdir() if p.is_dir()):
        if scenario_dir.name in {"regression", "__pycache__", "cli"}:
            continue
        for source in sorted(scenario_dir.rglob("*.bas")):
            rel = source.relative_to(scenario_dir).with_suffix("").as_posix()
            programs.append(Program(scenario_dir.name, rel, source))
    return programs


def select_programs(
    programs: list[Program], scenario: str | None, program: str | None
) -> list[Program]:
    selection = programs
    if scenario:
        selection = [p for p in selection if p.scenario == scenario]
        if not selection:
            names = sorted({p.scenario for p in programs})
            fail(f"unknown scenario {scenario!r}; available: {', '.join(names)}")
    if program:
        matched = [
            p
            for p in selection
            if program in {p.stem, p.rel, f"{p.scenario}/{p.stem}", f"{p.scenario}/{p.rel}"}
        ]
        if not matched:
            names = sorted({p.stem for p in selection})
            fail(f"unknown program {program!r}; available: {', '.join(names)}")
        selection = matched
    if not selection:
        fail("no programs selected")
    return selection


def scenario_config(manifest: dict[str, Any], scenario: str) -> dict[str, Any]:
    entry = manifest.get("scenarios", {}).get(scenario, {})
    if not isinstance(entry, dict):
        fail(f"scenario config for {scenario!r} must be an object")
    return entry


def program_config(
    manifest: dict[str, Any], program: Program
) -> dict[str, Any]:
    defaults = dict(manifest.get("defaults", {}))
    scenario = scenario_config(manifest, program.scenario)
    scenario_programs = scenario.get("programs", {})
    if not isinstance(scenario_programs, dict):
        fail(f"scenario {program.scenario!r} programs must be an object")
    overrides = {}
    for key in (program.stem, program.rel):
        if key in scenario_programs:
            overrides = scenario_programs[key]
            break
    if not isinstance(overrides, dict):
        fail(f"program config for {program.label!r} must be an object")
    result = dict(defaults)
    for key, value in scenario.items():
        if key != "programs":
            result[key] = value
    result.update(overrides)
    result["_explicit_volatile"] = "volatile" in overrides
    return result


def screen_modes(text: str) -> tuple[int, int]:
    modes = [int(m.group(1)) for m in re.finditer(r"\bSCREEN\s*(\d+)\b", text, re.IGNORECASE)]
    if not modes:
        return 0, 0
    return modes[-1], max(modes)


def has_turbo_command(text: str) -> bool:
    return bool(
        re.search(r"\b(?:CMD|CALL)\s+TURBO\b|\bTURBO\s*\(", text, re.IGNORECASE)
    )


def infer_machine_class(text: str) -> str:
    _, maximum = screen_modes(text)
    if maximum >= 4 or has_turbo_command(text):
        return "msx2plus"
    return "msx1"


def normalise_machine_class(value: Any) -> str:
    text = str(value).strip()
    for machine_class in MACHINE_CLASSES:
        if text.lower() == machine_class.lower():
            return machine_class
    fail(f"unknown machine class {value!r}; expected one of {', '.join(MACHINE_CLASSES)}")


def machine_class_for(
    program: Program, config: dict[str, Any], text: str | None = None
) -> str:
    explicit = config.get("requires")
    if explicit:
        return normalise_machine_class(explicit)
    if text is None:
        text = program.source.read_text(encoding="utf-8", errors="replace")
    return infer_machine_class(text)


def machine_for_class(machine_class: str, config: dict[str, Any]) -> str:
    key = MACHINE_CLASS_KEYS[machine_class]
    return str(config.get(key, DEFAULTS[key]))


def machine_for(
    program: Program, config: dict[str, Any], text: str | None = None
) -> str:
    explicit = config.get("machine")
    if explicit:
        return str(explicit)
    return machine_for_class(machine_class_for(program, config, text), config)


def machine_boot_seconds(config: dict[str, Any], machine_class: str) -> float:
    minimums = DEFAULTS["machine_boot_seconds"]
    configured = config.get("machine_boot_seconds")
    if configured is None:
        configured = {}
    if not isinstance(configured, dict):
        fail("machine_boot_seconds must be an object")
    value = configured.get(machine_class, minimums[machine_class])
    try:
        seconds = float(value)
    except (TypeError, ValueError):
        fail(f"machine_boot_seconds[{machine_class!r}] is not a number: {value!r}")
    if seconds < 0:
        fail(f"machine_boot_seconds[{machine_class!r}] must be non-negative")
    return seconds


def effective_fallback_seconds(
    config: dict[str, Any], machine_class: str
) -> float:
    fallback = config.get("fallback_seconds", DEFAULTS["fallback_seconds"])
    try:
        configured = float(fallback)
    except (TypeError, ValueError):
        fail(f"fallback_seconds is not a number: {fallback!r}")
    if configured < 0:
        fail("fallback_seconds must be non-negative")
    return max(configured, machine_boot_seconds(config, machine_class))


def parse_noi(path: Path) -> dict[str, int]:
    symbols: dict[str, int] = {}
    if not path.exists():
        return symbols
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = re.match(r"\s*def\s+(\S+)\s+([0-9A-Fa-f]+)H\b", line)
        if match:
            symbols[match.group(1)] = int(match.group(2), 16)
    return symbols


def breakpoints_from_symbols(symbols: dict[str, int]) -> list[tuple[str, int]]:
    # GRAPH/line1 compiles to END_STMT=0x8063, whose first two bytes are a JR
    # skip; END_STMT+2=0x8065 is the CALL DISP_XBASIC_END instruction before
    # the text-mode restore. END_PGM=0x828B jumps to 0x8065, so explicit and
    # implicit ends reach the same pre-cleanup instruction.
    if "END_STMT" in symbols:
        return [("END_STMT+2", (symbols["END_STMT"] & 0xFFFF) + 2)]
    if "END_PGM" in symbols:
        return [("END_PGM", symbols["END_PGM"] & 0xFFFF)]
    return []


def breakpoints_for(noi_path: Path) -> list[tuple[str, int]]:
    return breakpoints_from_symbols(parse_noi(noi_path))



def is_volatile_program(config: dict[str, Any], text: str) -> bool:
    if config.get("_explicit_volatile"):
        return bool(config.get("volatile", False))
    if bool(config.get("volatile", False)):
        return True
    return bool(re.search(r"\b(RND|TIME|TIME\$|DATE\$|INTERVAL)\b", text, re.IGNORECASE))


def compile_program(program: Program, outdir: Path) -> tuple[Path | None, Path | None, str]:
    outdir.mkdir(parents=True, exist_ok=True)
    command = [
        str(APP),
        "-q",
        "-x",
        "-s",
        "--noi",
        "-o",
        str(outdir),
        str(program.source),
    ]
    try:
        result = subprocess.run(
            command, capture_output=True, text=True, timeout=180, check=False
        )
    except FileNotFoundError:
        fail(f"compiler not found: {APP}. Run 'make release' first.")
    except subprocess.TimeoutExpired:
        return None, None, "compiler timed out"
    rom = outdir / f"{program.stem}[ASCII8].rom"
    noi = outdir / f"{program.stem}[ASCII8].noi"
    output = (result.stdout + result.stderr).strip()
    if result.returncode != 0 or not rom.exists():
        return None, None, output or f"compiler exited with {result.returncode}"
    return rom, noi if noi.exists() else None, output


def read_capture_meta(path: Path) -> dict[str, str]:
    meta: dict[str, str] = {}
    if not path.exists():
        return meta
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            meta[key.strip()] = value.strip()
    return meta


def format_events(events: Iterable[dict[str, Any]]) -> list[str]:
    lines: list[str] = []
    for index, event in enumerate(events):
        if not isinstance(event, dict):
            fail(f"input event {index} must be an object")
        if "time" not in event:
            fail(f"input event {index} is missing 'time'")
        when = event["time"]
        if not isinstance(when, (int, float)) or when < 0:
            fail(f"input event {index} has invalid time: {when!r}")
        if "text" in event:
            lines.append(f"type {when} {event['text']}")
        elif "key" in event:
            duration = event.get("duration", 0.1)
            lines.append(f"key {when} {event['key']} {duration}")
        elif "joy" in event:
            state = event.get("state", "down")
            lines.append(f"joy {when} {event['joy']} {state}")
        elif "button" in event:
            state = event.get("state", "down")
            lines.append(f"joybutton {when} {event['button']} {state}")
        else:
            fail(f"input event {index} must contain text, key, joy, or button")
    return lines


def run_skill(
    rom: Path,
    workdir: Path,
    config: dict[str, Any],
    breakpoints: list[tuple[str, int]],
    input_events: list[dict[str, Any]],
    screenshot: bool,
) -> tuple[bool, str]:
    env = os.environ.copy()
    env["MSX_DEBUG_MACHINE"] = str(config["machine"])
    env["MSX_DEBUG_DELAY"] = str(config.get("fallback_seconds", DEFAULTS["fallback_seconds"]))
    env["MSX_DEBUG_VRAM"] = "1"
    env["MSX_DEBUG_SCREENSHOT"] = "1" if screenshot else "0"

    # Keyboard and joystick events are applied through host-time input. Keep
    # emulation at real time when an input script is present so characters and
    # XTEST events are not outrun.
    env["MSX_DEBUG_THROTTLE"] = "on" if input_events else "off"

    if breakpoints:
        env["MSX_DEBUG_BREAKPOINTS"] = ",".join(
            f"{label}=0x{address:04X}" for label, address in breakpoints
        )

    input_path = None
    if input_events:
        input_path = workdir / "input.txt"
        input_path.write_text("\n".join(format_events(input_events)) + "\n", encoding="utf-8")
        env["MSX_DEBUG_INPUT"] = str(input_path)

    # Clear any host key left pressed by an interrupted previous run.
    try:
        subprocess.run(
            [sys.executable, str(XTST_HELPER), "release_all"],
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=10,
            check=False,
        )
    except Exception:
        pass

    log_path = workdir / "emulator.log"
    try:
        with log_path.open("w", encoding="utf-8") as log:
            result = subprocess.run(
                [str(SKILL), str(rom)],
                stdout=log,
                stderr=subprocess.STDOUT,
                env=env,
                timeout=180,
                check=False,
            )
    except subprocess.TimeoutExpired:
        return False, "emulator timed out"
    except FileNotFoundError:
        fail(f"skill launcher not found: {SKILL}")

    if result.returncode != 0:
        tail = log_path.read_text(encoding="utf-8", errors="replace").strip().splitlines()[-5:]
        return False, "emulator failed: " + " | ".join(tail)
    return True, ""


def artifact_paths(rom: Path) -> dict[str, Path]:
    base = rom.name[:-4]
    directory = rom.parent
    return {
        "ram": directory / f"{base}.ram-c000-ffff.bin",
        "vram": directory / f"{base}.vram.bin",
        "vdp_regs": directory / f"{base}.vdp-regs.bin",
        "screenshot": directory / f"{base}.screen.png",
        "meta": directory / f"{base}.capture.txt",
    }



V9958_SCREEN_MODES = {
    (0x00, 0x10): 0,
    (0x00, 0x00): 1,
    (0x02, 0x00): 2,
    (0x00, 0x08): 3,
    (0x04, 0x00): 4,
    (0x06, 0x00): 5,
    (0x08, 0x00): 6,
    (0x0A, 0x00): 7,
    (0x0E, 0x00): 8,
}


def screen_from_vdp(regs: bytes | None, fallback: int) -> int:
    if not regs or len(regs) < 2:
        return fallback
    key = (regs[0] & 0x0E, regs[1] & 0x18)
    if key == (0x0E, 0x00):
        return fallback if 9 <= fallback <= 12 else 8
    return V9958_SCREEN_MODES.get(key, fallback)


def screen_mode_consistency(
    source_screen: int | None, vdp_screen: int
) -> tuple[int, str | None]:
    if source_screen is None or source_screen == vdp_screen:
        return vdp_screen, None
    layout_screen = vdp_screen
    if vdp_screen in {0, 1} and source_screen not in {0, 1}:
        layout_screen = source_screen
    message = (
        f"source SCREEN {source_screen} conflicts with captured VDP mode "
        f"{vdp_screen}; using SCREEN {layout_screen} for VRAM layout"
    )
    return layout_screen, message


def extract_text(
    vram: bytes,
    ram: bytes,
    screen: int,
    config: dict[str, Any],
) -> str:
    text_config = config.get("text", {})
    name_table = text_config.get("name_table")
    if name_table is None:
        offset = 0xF922 - RAM_START
        if 0 <= offset + 1 < len(ram):
            name_table = ram[offset] | (ram[offset + 1] << 8)
        else:
            name_table = 0
        if name_table >= len(vram):
            name_table = 0x0000 if screen == 0 else 0x1800
    name_table = parse_int(name_table, "vram.text.name_table")

    width = parse_int(text_config.get("width", 40 if screen == 0 else 32), "vram.text.width")
    height = parse_int(text_config.get("height", 24), "vram.text.height")
    if width <= 0 or height <= 0:
        fail("vram.text width and height must be positive")

    rows: list[str] = []
    for row in range(height):
        chars: list[str] = []
        for column in range(width):
            index = name_table + row * width + column
            if index >= len(vram):
                break
            code = vram[index]
            if 0x20 <= code <= 0x7E:
                chars.append(chr(code))
            elif code == 0:
                chars.append(" ")
            else:
                chars.append(".")
        rows.append("".join(chars).rstrip())
    while rows and not rows[-1].strip():
        rows.pop()
    return "\n".join(rows)


def normalise_text(text: str) -> str:
    # Name-table rows wrap at the current text width, so compare without
    # whitespace: wrapping must not change the expected text result.
    return "".join(text.split())


def region_specs(screen: int, vram_size: int, config: dict[str, Any]) -> list[tuple[str, int, int]]:
    configured = config.get("regions")
    if configured:
        specs: list[tuple[str, int, int]] = []
        for index, region in enumerate(configured):
            if not isinstance(region, dict) or "name" not in region:
                fail(f"vram region {index} must be an object with a name")
            start = parse_int(region.get("start", 0), f"vram region {region['name']} start")
            length = parse_int(region.get("length", 0), f"vram region {region['name']} length")
            if length <= 0:
                fail(f"vram region {region['name']} length must be positive")
            if start + length > vram_size:
                fail(f"vram region {region['name']} exceeds captured VRAM ({vram_size} bytes)")
            specs.append((str(region["name"]), start, length))
        return specs

    if screen == 2:
        return [(name, start, min(length, vram_size - start))
                for name, start, length in SCREEN2_REGIONS if start < vram_size]
    if screen in {0, 1}:
        return []
    return [("bitmap", 0, vram_size)]


def sha256_region(data: bytes, start: int, length: int) -> str:
    return hashlib.sha256(data[start:start + length]).hexdigest()


def compress_bytes(data: bytes) -> str:
    return base64.b64encode(zlib.compress(data, 9)).decode("ascii")


def decompress_bytes(data: str) -> bytes:
    return zlib.decompress(base64.b64decode(data))


def volatile_offsets(ranges: Iterable[tuple[int, int]]) -> set[int]:
    offsets: set[int] = set()
    for start, end in ranges:
        for address in range(max(start, RAM_START), min(end, RAM_END) + 1):
            offsets.add(address - RAM_START)
    return offsets


def build_baseline(
    program: Program,
    config: dict[str, Any],
    ram_runs: list[bytes],
    vram_runs: list[bytes],
    meta: dict[str, str],
    screens: list[int],
    runs: int,
) -> dict[str, Any]:
    ranges = [parse_range(item, f"{program.label} volatile_ranges")
              for item in config.get("volatile_ranges", [])]
    volatile = bool(config.get("volatile", False))
    screen = screens[0]
    screen_stable = len(set(screens)) == 1

    baseline: dict[str, Any] = {
        "version": 1,
        "program": program.label,
        "scenario": program.scenario,
        "source": str(program.source.relative_to(ROOT)),
        "machine": str(config["machine"]),
        "machine_class": config.get("machine_class"),
        "effective_fallback_seconds": config.get(
            "fallback_seconds", DEFAULTS["fallback_seconds"]
        ),
        "screen": screen,
        "screen_stable": screen_stable,
        "volatile": volatile,
        "runs": runs,
        "generated_at": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "capture_trigger": meta.get("trigger"),
        "capture_time": meta.get("emulated_time"),
        "ram": None,
        "vram": None,
        "probes": config.get("ram_probes", []),
    }

    if not volatile:
        offsets = volatile_offsets(ranges)
        mask = bytearray(RAM_SIZE)
        stable = bytearray(RAM_SIZE)
        for offset in range(RAM_SIZE):
            if offset in offsets:
                continue
            values = {run[offset] for run in ram_runs}
            if len(values) == 1:
                mask[offset] = 0xFF
                stable[offset] = ram_runs[0][offset]
        baseline["ram"] = {
            "mask": compress_bytes(bytes(mask)),
            "values": compress_bytes(bytes(stable)),
            "volatile_ranges": [[start, end] for start, end in ranges],
        }

    text_config = config.get("vram", {}).get("text") if isinstance(config.get("vram"), dict) else None
    vram_baseline: dict[str, Any] = {
        "size": len(vram_runs[0]),
        "screen_stable": screen_stable,
    }
    if screen_stable and screen in {0, 1}:
        texts = [
            extract_text(vram, ram, screen, config.get("vram", {}))
            for vram, ram in zip(vram_runs, ram_runs)
        ]
        if len(set(texts)) == 1:
            vram_baseline["text"] = texts[0]
        else:
            vram_baseline["text_stable"] = False
        if isinstance(text_config, dict) and text_config.get("expected") is not None:
            vram_baseline["expected_text"] = text_config["expected"]

    if screen_stable:
        regions = region_specs(screen, len(vram_runs[0]), config.get("vram", {}))
    else:
        regions = []
    stable_regions: dict[str, Any] = {}
    unstable_regions: list[str] = []
    if regions and not volatile:
        for name, start, length in regions:
            hashes = {sha256_region(vram, start, length) for vram in vram_runs}
            if len(hashes) == 1:
                stable_regions[name] = {
                    "start": start,
                    "length": length,
                    "sha256": hashes.pop(),
                }
            else:
                unstable_regions.append(name)
    vram_baseline["regions"] = stable_regions
    vram_baseline["unstable_regions"] = unstable_regions
    baseline["vram"] = vram_baseline
    return baseline


def compare_ram(
    program: Program, config: dict[str, Any], baseline: dict[str, Any], ram: bytes
) -> tuple[bool, str, list[dict[str, Any]]]:
    ram_baseline = baseline.get("ram")
    if not ram_baseline:
        return True, "", []

    mask = decompress_bytes(ram_baseline["mask"])
    expected = decompress_bytes(ram_baseline["values"])
    if len(mask) != RAM_SIZE or len(expected) != RAM_SIZE:
        return False, "baseline RAM size does not match captured RAM size", []

    ranges = [parse_range(item, f"{program.label} volatile_ranges")
              for item in config.get("volatile_ranges", [])]
    ranges += [tuple(item) for item in ram_baseline.get("volatile_ranges", [])]
    ignored = volatile_offsets(ranges)

    diffs: list[int] = []
    for offset in range(RAM_SIZE):
        if offset in ignored:
            continue
        if mask[offset] and ram[offset] != expected[offset]:
            diffs.append(offset)
    if not diffs:
        return True, "", []

    ranges: list[tuple[int, int]] = []
    start = previous = diffs[0]
    for offset in diffs[1:]:
        if offset == previous + 1:
            previous = offset
            continue
        ranges.append((start, previous))
        start = previous = offset
    ranges.append((start, previous))

    details = [
        {
            "name": "ram_snapshot",
            "ok": False,
            "detail": (
                f"{len(diffs)} differing byte(s); first range "
                f"0x{RAM_START + ranges[0][0]:04X}-0x{RAM_START + ranges[0][1]:04X}"
            ),
        }
    ]
    return False, details[0]["detail"], details


def compare_probes(
    config: dict[str, Any], ram: bytes
) -> tuple[bool, list[dict[str, Any]]]:
    checks: list[dict[str, Any]] = []
    all_ok = True
    for index, probe in enumerate(config.get("ram_probes", [])):
        if not isinstance(probe, dict) or "address" not in probe:
            fail(f"ram probe {index} must be an object with an address")
        address = parse_int(probe["address"], f"ram probe {index} address")
        if not (RAM_START <= address <= RAM_END):
            fail(f"ram probe {index} address 0x{address:04X} is outside 0xC000-0xFFFF")
        offset = address - RAM_START
        if "bytes" in probe:
            raw = bytes(
                int(part, 16) if isinstance(part, str) else int(part)
                for part in re.split(r"[\s,]+", str(probe["bytes"]).strip())
                if part
            )
        elif "value" in probe:
            raw = bytes([parse_int(probe["value"], f"ram probe {index} value") & 0xFF])
        else:
            fail(f"ram probe {index} must contain value or bytes")
        actual = ram[offset:offset + len(raw)]
        ok = actual == raw
        all_ok = all_ok and ok
        expected_text = " ".join(f"{byte:02X}" for byte in raw)
        actual_text = " ".join(f"{byte:02X}" for byte in actual)
        checks.append({
            "name": f"ram_probe_0x{address:04X}",
            "ok": ok,
            "detail": f"expected {expected_text}, got {actual_text}",
        })
    return all_ok, checks


def check_baseline_machine(
    config: dict[str, Any], baseline: dict[str, Any]
) -> tuple[bool, str]:
    expected = baseline.get("machine")
    selected = str(config.get("machine", ""))
    if expected and str(expected) != selected:
        return False, (
            f"baseline machine {expected} does not match selected machine "
            f"{selected}; regenerate the baseline"
        )
    return True, ""


def compare_vram_probes(
    config: dict[str, Any], vram: bytes
) -> tuple[bool, list[dict[str, Any]]]:
    checks: list[dict[str, Any]] = []
    all_ok = True
    for index, probe in enumerate(config.get("vram", {}).get("probes", [])):
        if not isinstance(probe, dict) or "address" not in probe:
            fail(f"vram probe {index} must be an object with an address")
        address = parse_int(probe["address"], f"vram probe {index} address")
        if "bytes" in probe:
            raw = bytes(
                int(part, 16) if isinstance(part, str) else int(part)
                for part in re.split(r"[\s,]+", str(probe["bytes"]).strip())
                if part
            )
        elif "value" in probe:
            raw = bytes([parse_int(probe["value"], f"vram probe {index} value") & 0xFF])
        else:
            fail(f"vram probe {index} must contain value or bytes")
        if address < 0 or address >= len(vram):
            fail(
                f"vram probe {index} address 0x{address:04X} is outside "
                f"captured VRAM 0x0000-0x{len(vram) - 1:04X}"
            )
        if address + len(raw) > len(vram):
            fail(
                f"vram probe {index} range 0x{address:04X}-"
                f"0x{address + len(raw) - 1:04X} exceeds captured VRAM "
                f"size 0x{len(vram):04X}"
            )
        actual = vram[address:address + len(raw)]
        ok = actual == raw
        all_ok = all_ok and ok
        expected_text = " ".join(f"{byte:02X}" for byte in raw)
        actual_text = " ".join(f"{byte:02X}" for byte in actual)
        checks.append({
            "name": f"vram_probe_0x{address:04X}",
            "ok": ok,
            "detail": f"expected {expected_text}, got {actual_text}",
        })
    return all_ok, checks


def compare_vram(
    program: Program,
    config: dict[str, Any],
    baseline: dict[str, Any],
    vram: bytes,
    ram: bytes,
    screen: int,
    volatile: bool,
) -> tuple[bool, list[dict[str, Any]]]:
    checks: list[dict[str, Any]] = []
    all_ok = True
    vram_config = config.get("vram", {})
    if not isinstance(vram_config, dict):
        fail(f"{program.label} vram config must be an object")

    baseline_vram = baseline.get("vram") or {}
    baseline_screen = baseline.get("screen")
    screen_stable = baseline_vram.get("screen_stable", True)
    text_config = vram_config.get("text")
    configured_text = (
        text_config.get("expected") if isinstance(text_config, dict) else None
    )
    configured_regions = bool(vram_config.get("regions"))

    mode_mismatch = baseline_screen is not None and baseline_screen != screen

    # Baseline text is kept for diagnostics only. Only manifest-configured
    # expected text participates in pass/fail so scrolling loops do not create
    # false failures.
    expected_text = configured_text

    if expected_text is not None:
        if screen not in {0, 1}:
            checks.append({
                "name": "vram_text",
                "ok": False,
                "detail": f"captured screen mode {screen} is not a text mode",
            })
            all_ok = False
        else:
            actual_text = extract_text(vram, ram, screen, vram_config)
            if isinstance(expected_text, str):
                expectations = [expected_text]
            else:
                expectations = list(expected_text)
            normal_actual = normalise_text(actual_text)
            missing = [
                text for text in expectations
                if normalise_text(str(text)) not in normal_actual
            ]
            ok = not missing
            all_ok = all_ok and ok
            detail = (
                "text matches" if ok
                else "missing text: " + " | ".join(str(item) for item in missing)
            )
            checks.append({"name": "vram_text", "ok": ok, "detail": detail})

    probe_ok, probe_checks = compare_vram_probes(config, vram)
    checks.extend(probe_checks)
    all_ok = all_ok and probe_ok

    specs = region_specs(screen, len(vram), vram_config)
    baseline_regions = baseline_vram.get("regions") or {}
    unstable_regions = set(baseline_vram.get("unstable_regions") or [])

    if volatile:
        return all_ok, checks

    if not specs:
        return all_ok, checks

    if not screen_stable or mode_mismatch:
        if configured_regions:
            for name, start, length in specs:
                expected = baseline_regions.get(name)
                if not expected:
                    checks.append({
                        "name": f"vram_region_{name}",
                        "ok": False,
                        "detail": "screen mode varied; no stable baseline region",
                    })
                    all_ok = False
                    continue
                actual_hash = sha256_region(vram, start, length)
                ok = actual_hash == expected.get("sha256")
                all_ok = all_ok and ok
                checks.append({
                    "name": f"vram_region_{name}",
                    "ok": ok,
                    "detail": (
                        f"region 0x{start:04X}-0x{start + length - 1:04X} "
                        f"{'matches' if ok else 'differs from baseline'}"
                    ),
                })
        else:
            checks.append({
                "name": "vram_info",
                "ok": True,
                "detail": "screen mode was not stable across baseline runs; VRAM comparison skipped",
            })
        return all_ok, checks

    for name, start, length in specs:
        expected = baseline_regions.get(name)
        if not expected:
            if name in unstable_regions:
                checks.append({
                    "name": "vram_info",
                    "ok": True,
                    "detail": f"region {name} was not stable across baseline runs; comparison skipped",
                })
            elif configured_regions:
                checks.append({
                    "name": f"vram_region_{name}",
                    "ok": False,
                    "detail": "missing baseline region",
                })
                all_ok = False
            else:
                checks.append({
                    "name": "vram_info",
                    "ok": True,
                    "detail": f"no stable baseline hash for region {name}; comparison skipped",
                })
            continue
        actual_hash = sha256_region(vram, start, length)
        ok = actual_hash == expected.get("sha256")
        all_ok = all_ok and ok
        checks.append({
            "name": f"vram_region_{name}",
            "ok": ok,
            "detail": (
                f"region 0x{start:04X}-0x{start + length - 1:04X} "
                f"{'matches' if ok else 'differs from baseline'}"
            ),
        })
    return all_ok, checks

def run_program(
    program: Program,
    config: dict[str, Any],
    run_dir: Path,
    screenshot_override: bool,
) -> dict[str, Any]:
    result: dict[str, Any] = {
        "program": program.label,
        "scenario": program.scenario,
        "source": str(program.source.relative_to(ROOT)),
        "status": "error",
        "decision": "none",
        "checks": [],
        "failure": None,
        "artifacts": {},
    }
    program_dir = run_dir / program.scenario / program.rel.replace("/", "__")
    build_dir = program_dir / "build"
    text = program.source.read_text(encoding="utf-8", errors="replace")
    screen, maximum = screen_modes(text)
    machine_class = machine_class_for(program, config, text)
    config["machine"] = machine_for(program, config, text)
    config["machine_class"] = machine_class
    config["fallback_seconds"] = effective_fallback_seconds(config, machine_class)
    result["machine_class"] = machine_class
    result["machine"] = config["machine"]
    result["effective_fallback_seconds"] = config["fallback_seconds"]

    rom, noi, compile_output = compile_program(program, build_dir)
    if rom is None:
        result["failure"] = f"compilation failed: {compile_output}"
        return result
    if compile_output:
        (program_dir / "compile.log").write_text(compile_output + "\n", encoding="utf-8")

    breakpoints = breakpoints_for(noi) if noi else []

    volatile = is_volatile_program(config, text)
    if volatile:
        config["volatile"] = True

    memory_checks = bool(
        config.get("ram_probes")
        or (isinstance(config.get("vram"), dict) and config.get("vram"))
        or not volatile
    )
    screenshot = screenshot_override or bool(config.get("screenshot", not memory_checks))

    input_events = config.get("input", [])
    if not isinstance(input_events, list):
        fail(f"{program.label} input must be a list")

    ok, error = run_skill(
        rom, program_dir, config, breakpoints, input_events, screenshot
    )
    paths = artifact_paths(rom)
    result["artifacts"] = {name: str(path.relative_to(ROOT)) for name, path in paths.items() if path.exists()}
    if not ok:
        result["failure"] = error
        return result
    if not paths["ram"].exists():
        result["failure"] = "emulator produced no RAM artifact"
        return result

    ram = paths["ram"].read_bytes()
    if len(ram) != RAM_SIZE:
        result["failure"] = f"RAM artifact has {len(ram)} bytes, expected {RAM_SIZE}"
        return result
    vram = paths["vram"].read_bytes() if paths["vram"].exists() else b""
    if not vram:
        result["failure"] = "emulator produced no VRAM artifact"
        return result
    vdp_regs = paths["vdp_regs"].read_bytes() if paths["vdp_regs"].exists() else None
    source_screen = screen if maximum > 0 else None
    vdp_screen = screen_from_vdp(vdp_regs, screen)
    layout_screen, screen_conflict = screen_mode_consistency(source_screen, vdp_screen)
    result["source_screen"] = source_screen
    result["vdp_screen"] = vdp_screen
    result["layout_screen"] = layout_screen
    if screen_conflict:
        result["screen_conflict"] = screen_conflict
    screen = layout_screen
    meta = read_capture_meta(paths["meta"])

    volatile = bool(config.get("volatile", False))

    baseline: dict[str, Any] = {}
    if not volatile:
        if not program.baseline_path.exists():
            result["failure"] = (
                f"missing baseline {program.baseline_path.relative_to(ROOT)}; "
                "run regression-baseline first"
            )
            return result
        baseline = json.loads(program.baseline_path.read_text(encoding="utf-8"))
        machine_ok, machine_reason = check_baseline_machine(config, baseline)
        if not machine_ok:
            result["checks"] = [{
                "name": "baseline_machine",
                "ok": False,
                "detail": machine_reason,
            }]
            result["failure"] = machine_reason
            result["status"] = "fail"
            result["decision"] = "memory"
            result["volatile"] = volatile
            result["capture_trigger"] = meta.get("trigger")
            return result

    checks: list[dict[str, Any]] = []
    all_ok = True
    memory_decision = False

    probe_ok, probe_checks = compare_probes(config, ram)
    checks.extend(probe_checks)
    all_ok = all_ok and probe_ok
    if probe_checks:
        memory_decision = True

    if volatile:
        checks.append({
            "name": "volatile_program",
            "ok": True,
            "detail": "RAM snapshot comparison skipped; configured probes still evaluated",
        })
    else:
        ram_ok, reason, ram_checks = compare_ram(program, config, baseline, ram)
        checks.extend(ram_checks)
        all_ok = all_ok and ram_ok
        memory_decision = True
        if not ram_ok:
            result["failure"] = reason

    vram_ok, vram_checks = compare_vram(
        program, config, baseline, vram, ram, screen, volatile
    )
    checks.extend(vram_checks)
    all_ok = all_ok and vram_ok
    if any(
        check["name"] == "vram_text"
        or check["name"].startswith("vram_region_")
        or check["name"].startswith("vram_probe_")
        for check in vram_checks
    ):
        memory_decision = True

    if not memory_decision:
        if paths["screenshot"].exists():
            checks.append({
                "name": "screenshot",
                "ok": True,
                "detail": "no memory checks configured; decision is visual",
            })
            result["decision"] = "visual"
        else:
            checks.append({
                "name": "screenshot",
                "ok": False,
                "detail": "no memory checks configured and no screenshot captured",
            })
            all_ok = False
            result["failure"] = "no memory checks and no screenshot"
    else:
        result["decision"] = "memory"

    if not all_ok and result["failure"] is None:
        result["failure"] = next(
            (check["detail"] for check in checks if not check["ok"]),
            "one or more checks failed",
        )

    result["checks"] = checks
    result["status"] = "pass" if all_ok else "fail"
    result["volatile"] = volatile
    result["capture_trigger"] = meta.get("trigger")
    return result


def write_reports(run_dir: Path, results: list[dict[str, Any]], started: str) -> None:
    summary = {
        "total": len(results),
        "passed": sum(1 for item in results if item["status"] == "pass"),
        "failed": sum(1 for item in results if item["status"] == "fail"),
        "errors": sum(1 for item in results if item["status"] == "error"),
    }
    report = {
        "generated_at": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "started_at": started,
        "summary": summary,
        "results": results,
    }
    (run_dir / "report.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )

    lines = [
        "=== MSX Regression Report ===",
        f"Started: {started}",
        f"Total: {summary['total']}  Passed: {summary['passed']}  "
        f"Failed: {summary['failed']}  Errors: {summary['errors']}",
        "",
    ]
    for item in results:
        lines.append(
            f"{item['status'].upper():5} {item['program']} "
            f"({item.get('decision', 'none')}) "
            f"[{item.get('machine_class', '?')}] {item.get('machine', '?')} "
            f"fallback={item.get('effective_fallback_seconds', '?')}s"
        )
        if item.get("failure"):
            lines.append(f"      failure: {item['failure']}")
        if item.get("screen_conflict"):
            lines.append(f"      note: {item['screen_conflict']}")
        for check in item.get("checks", []):
            if not check.get("ok", True):
                lines.append(f"      {check['name']}: {check['detail']}")
    (run_dir / "report.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")


def command_list(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    programs = select_programs(discover_programs(), args.scenario, args.program)
    current = None
    for program in programs:
        if program.scenario != current:
            current = program.scenario
            print(f"{current}:")
        config = program_config(manifest, program)
        text = program.source.read_text(encoding="utf-8", errors="replace")
        machine_class = machine_class_for(program, config, text)
        machine = machine_for(program, config, text)
        print(f"  {program.rel} [{machine_class}] {machine}")
    return 0


def command_run(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    programs = select_programs(discover_programs(), args.scenario, args.program)
    run_id = time.strftime("%Y%m%d-%H%M%S")
    run_dir = ARTIFACTS / "runs" / run_id
    latest = ARTIFACTS / "runs" / "latest"
    if latest.is_symlink() or latest.is_file():
        latest.unlink()
    elif latest.exists():
        shutil.rmtree(latest)
    run_dir.mkdir(parents=True, exist_ok=True)
    started = time.strftime("%Y-%m-%dT%H:%M:%S%z")

    results: list[dict[str, Any]] = []
    for index, program in enumerate(programs, 1):
        print(f"[{index}/{len(programs)}] {program.label}")
        config = program_config(manifest, program)
        result = run_program(program, config, run_dir, args.screenshots)
        results.append(result)
        print(f"    {result['status'].upper()}" + (f" - {result['failure']}" if result.get('failure') else ""))

    write_reports(run_dir, results, started)
    if latest.exists() or latest.is_symlink():
        latest.unlink()
    latest.symlink_to(run_dir.name)
    summary = json.loads((run_dir / "report.json").read_text(encoding="utf-8"))["summary"]
    print(
        f"\nReport: {run_dir / 'report.txt'}\n"
        f"Total {summary['total']} | pass {summary['passed']} | "
        f"fail {summary['failed']} | error {summary['errors']}"
    )
    return 0 if summary["failed"] == 0 and summary["errors"] == 0 else 1


def capture_once(
    program: Program,
    config: dict[str, Any],
    base_dir: Path,
    run_number: int,
    screenshot_override: bool,
) -> tuple[bytes | None, bytes | None, dict[str, str], str]:
    workdir = base_dir / program.scenario / program.rel.replace("/", "__") / f"run{run_number}"
    build_dir = workdir / "build"
    rom, noi, compile_output = compile_program(program, build_dir)
    if rom is None:
        return None, None, {}, f"compilation failed: {compile_output}"

    text = program.source.read_text(encoding="utf-8", errors="replace")
    screen, _ = screen_modes(text)
    machine_class = machine_class_for(program, config, text)
    config["machine"] = machine_for(program, config, text)
    config["machine_class"] = machine_class
    config["fallback_seconds"] = effective_fallback_seconds(config, machine_class)
    breakpoints = breakpoints_for(noi) if noi else []
    screenshot = screenshot_override or bool(config.get("screenshot", False))
    input_events = config.get("input", [])
    if not isinstance(input_events, list):
        fail(f"{program.label} input must be a list")

    ok, error = run_skill(
        rom, workdir, config, breakpoints, input_events, screenshot
    )
    if not ok:
        return None, None, None, {}, error
    paths = artifact_paths(rom)
    if not paths["ram"].exists() or not paths["vram"].exists():
        return None, None, None, {}, "emulator produced incomplete artifacts"
    vdp_regs = paths["vdp_regs"].read_bytes() if paths["vdp_regs"].exists() else None
    return (
        paths["ram"].read_bytes(),
        paths["vram"].read_bytes(),
        vdp_regs,
        read_capture_meta(paths["meta"]),
        "",
    )


def command_baseline(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    programs = select_programs(discover_programs(), args.scenario, args.program)
    runs = args.runs
    if runs < 2:
        fail("baseline requires at least two runs")
    run_id = time.strftime("%Y%m%d-%H%M%S")
    base_dir = ARTIFACTS / "baseline-runs" / run_id
    base_dir.mkdir(parents=True, exist_ok=True)

    failures = 0
    for index, program in enumerate(programs, 1):
        print(f"[{index}/{len(programs)}] {program.label} x{runs}")
        config = program_config(manifest, program)
        text = program.source.read_text(encoding="utf-8", errors="replace")
        if is_volatile_program(config, text):
            config["volatile"] = True
        screen, maximum = screen_modes(text)
        source_screen = screen if maximum > 0 else None
        ram_runs: list[bytes] = []
        vram_runs: list[bytes] = []
        meta: dict[str, str] = {}
        screens: list[int] = []
        error = ""
        for run_number in range(1, runs + 1):
            ram, vram, vdp_regs, run_meta, error = capture_once(
                program, config, base_dir, run_number, False
            )
            if ram is None or vram is None:
                break
            ram_runs.append(ram)
            vram_runs.append(vram)
            selected_screen, _ = screen_mode_consistency(
                source_screen, screen_from_vdp(vdp_regs, screen)
            )
            screens.append(selected_screen)
            meta = meta or run_meta

        if len(ram_runs) != runs:
            print(f"    ERROR - {error or 'incomplete capture'}")
            failures += 1
            continue

        baseline = build_baseline(
            program, config, ram_runs, vram_runs, meta, screens, runs
        )
        program.baseline_path.parent.mkdir(parents=True, exist_ok=True)
        program.baseline_path.write_text(
            json.dumps(baseline, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
        print(f"    baseline: {program.baseline_path.relative_to(ROOT)}")

    shutil.rmtree(base_dir, ignore_errors=True)
    if failures:
        print(f"{failures} program(s) failed to produce a baseline", file=sys.stderr)
        return 1
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    for name in ("list", "run", "baseline"):
        sub = subparsers.add_parser(name)
        sub.add_argument("--scenario", help="feature folder to select")
        sub.add_argument("--program", help="program stem or scenario/program")
        if name == "run":
            sub.add_argument(
                "--screenshots",
                action="store_true",
                help="capture screenshots even when memory checks exist",
            )
        if name == "baseline":
            sub.add_argument(
                "--runs",
                type=int,
                default=None,
                help="number of captures per program (default: manifest baseline_runs)",
            )

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if args.command == "list":
        return command_list(args)
    if args.command == "run":
        return command_run(args)
    if args.command == "baseline":
        if args.runs is None:
            manifest = load_manifest()
            args.runs = int(manifest["defaults"].get("baseline_runs", DEFAULTS["baseline_runs"]))
        return command_baseline(args)
    parser.error("unknown command")
    return 2


if __name__ == "__main__":
    sys.exit(main())
