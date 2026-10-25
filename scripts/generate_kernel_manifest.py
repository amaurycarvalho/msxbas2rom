#!/usr/bin/env python3
"""Generate the kernel module manifest and baseline size map.

The manifest is derived from the assembled kernel symbols and the source files
that define each symbol.  It is intentionally a build-time artifact: BASIC
compilation never parses assembly sources or invokes an assembler.
"""

import argparse
import json
import re
import sys
from pathlib import Path

LABEL_RE = re.compile(
    r"^\s*([A-Za-z_.][A-Za-z0-9_.]*)\s*:\s*([A-Za-z0-9_./=]*)"
)
SYMBOL_RE = re.compile(
    r"^([A-Za-z_.$][A-Za-z0-9_.$]*)\s+EQU\s+(?:0x)?([0-9A-Fa-f]+)H?\s*$"
)

KERNEL_START = 0x4000
KERNEL_END = 0x8000
PLAYER_INTERRUPT_NEXT = "cmd_plyload"


def read_text(path):
    return path.read_text(encoding="utf-8", errors="replace")


def collect_source_labels(header_dir):
    labels = {}
    for path in sorted(header_dir.glob("*.asm")):
        owner = path.stem
        for line in read_text(path).splitlines():
            match = LABEL_RE.match(line)
            if not match:
                continue
            remainder = match.group(2).lower()
            if remainder in ("equ", "="):
                continue
            labels.setdefault(match.group(1), owner)
    return labels


TRANSFER_RE = re.compile(
    r"\b(?:call|jp|jr)\s+(?:(?:n?z|n?c|p[oe]?|m)\s*,\s*)?"
    r"([A-Za-z_.][A-Za-z0-9_.]*)",
    re.IGNORECASE,
)
CONDITION = r"(?:n?z|n?c|p[oe]?|m)"
SOURCE_TRANSFER_RE = re.compile(
    r"^\s*(call|jp|jr)\s+(?:(?P<cond>" + CONDITION + r")\s*,\s*)?"
    r"(?P<target>[A-Za-z_.][\w.]*)\b",
    re.IGNORECASE,
)
ALIAS_RE = re.compile(
    r"^\s*([A-Za-z_.$][\w.$]*)\s*:?\s*equ\s+"
    r"([A-Za-z_.$][\w.$]*)\s*(?:;.*)?$",
    re.IGNORECASE,
)
LABEL_ONLY_RE = re.compile(
    r"^\s*[A-Za-z_.][\w.]*\s*:\s*(?:;.*)?$"
)
LEADING_LABEL_RE = re.compile(r"^\s*[A-Za-z_.][\w.]*\s*:\s*")
TOKEN_RE = re.compile(r"(?<![.\w])[A-Za-z_.][\w.]*")
IMMOVABLE_CLASSIFICATIONS = ("fixed", "pinned")
IMPORT_KIND_NUMBERS = {
    "call": 0,
    "jump": 1,
    "conditional": 2,
    "address": 3,
}
CALL_OPCODES = {
    0xCD, 0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC
}
JUMP_OPCODES = {
    0xC3, 0xC2, 0xCA, 0xD2, 0xDA, 0xE2, 0xEA, 0xF2, 0xFA
}
ADDRESS_OPCODES = {0x01, 0x11, 0x21, 0x31, 0x22, 0x2A, 0x32, 0x3A}
INDEXED_ADDRESS_OPCODES = {0x21, 0x22, 0x2A, 0x36}
EXTENDED_ADDRESS_OPCODES = {
    0x43, 0x4B, 0x53, 0x5B, 0x63, 0x6B, 0x73, 0x7B
}


def collect_aliases(header_dir, label_owners):
    aliases = {}
    for path in sorted(header_dir.glob("*.asm")):
        for line in read_text(path).splitlines():
            match = ALIAS_RE.match(line)
            if not match:
                continue
            name, target = match.group(1), match.group(2)
            if target in label_owners:
                aliases[name] = target
    return aliases


def resolve_label(name, label_owners, aliases, seen=None):
    if seen is None:
        seen = set()
    if name in seen:
        return None
    seen.add(name)
    if name in label_owners:
        return name
    if name in aliases:
        return resolve_label(aliases[name], label_owners, aliases, seen)
    return None


def cross_module_references(owner, source_text, label_owners):
    references = []
    for match in TRANSFER_RE.finditer(source_text):
        target = match.group(1)
        if target.startswith("."):
            continue
        target_owner = label_owners.get(target)
        if target_owner and target_owner != owner:
            references.append((target, target_owner))
    return references


def source_module_references(
    owner,
    source_text,
    label_owners,
    aliases,
    classification_by_owner=None,
    movable_only=False,
    include_addresses=False,
):
    references = []
    for line in source_text.splitlines():
        code = line.split(";")[0]
        if not code.strip():
            continue
        if LABEL_ONLY_RE.match(code):
            continue
        if re.match(
            r"^\s*[A-Za-z_.$][\w.$]*\s*:?\s*equ\b", code, re.IGNORECASE
        ):
            continue

        instruction = code
        leading_label = LEADING_LABEL_RE.match(instruction)
        if leading_label:
            instruction = instruction[leading_label.end():]

        transfer = SOURCE_TRANSFER_RE.match(instruction)
        transfer_target = None
        kind = "address"
        if transfer:
            transfer_target = resolve_label(
                transfer.group("target"), label_owners, aliases
            )
            if transfer.group("cond"):
                kind = "conditional"
            elif transfer.group(1).lower() == "call":
                kind = "call"
            else:
                kind = "jump"
            if (
                transfer.group(1).lower() == "jr"
                and transfer_target
                and label_owners.get(transfer_target) not in (None, owner)
            ):
                raise ValueError(
                    f"{owner} uses jr to cross-module target "
                    f"'{transfer_target}'; use call/jp so the reference can "
                    "be fixed up"
                )

        for token in TOKEN_RE.findall(instruction):
            target = resolve_label(token, label_owners, aliases)
            if not target:
                continue
            target_owner = label_owners.get(target)
            if not target_owner or target_owner == owner:
                continue
            if movable_only and classification_by_owner:
                classification = classification_by_owner.get(
                    target_owner, "fixed"
                )
                if classification in IMMOVABLE_CLASSIFICATIONS:
                    continue
            if include_addresses:
                token_kind = kind if target == transfer_target else "address"
            else:
                if target != transfer_target:
                    continue
                token_kind = kind
            references.append((target, target_owner, token_kind))
    return references


def classify_import_opcode(module_bytes, offset):
    if offset >= 2:
        prefix = module_bytes[offset - 2]
        opcode = module_bytes[offset - 1]
        if prefix in (0xDD, 0xFD) and opcode in INDEXED_ADDRESS_OPCODES:
            return "address"
        if prefix == 0xED and opcode in EXTENDED_ADDRESS_OPCODES:
            return "address"
    if offset >= 1:
        opcode = module_bytes[offset - 1]
        if opcode in CALL_OPCODES:
            return "call" if opcode == 0xCD else "conditional"
        if opcode in JUMP_OPCODES:
            return "jump" if opcode == 0xC3 else "conditional"
        if opcode in ADDRESS_OPCODES:
            return "address"
    return None


def detect_module_imports(
    module,
    source_text,
    label_owners,
    aliases,
    classification_by_owner,
    symbols,
    kernel_bytes,
):
    if module["owner"] == "20_runtime":
        return []
    references = source_module_references(
        module["owner"],
        source_text,
        label_owners,
        aliases,
        classification_by_owner,
        movable_only=True,
        include_addresses=True,
    )
    if not references:
        return []

    for target, _, _ in references:
        if target not in symbols:
            raise ValueError(
                f"import target '{target}' has no assembled address"
            )

    target_addresses = {}
    for target, _, _ in references:
        target_addresses[symbols[target]] = target
    start = module["assembly_base"]
    module_bytes = kernel_bytes[start:start + module["size"]]

    found = []
    for offset in range(len(module_bytes) - 1):
        value = module_bytes[offset] | (module_bytes[offset + 1] << 8)
        target = target_addresses.get(value)
        if not target:
            continue
        kind = classify_import_opcode(module_bytes, offset)
        if kind is None:
            continue
        found.append({"offset": offset, "target": target, "kind": kind})

    expected = {}
    for target, _, kind in references:
        key = (target, kind)
        expected[key] = expected.get(key, 0) + 1
    actual = {}
    for entry in found:
        key = (entry["target"], entry["kind"])
        actual[key] = actual.get(key, 0) + 1
    missing = []
    for key, count in sorted(expected.items()):
        if actual.get(key, 0) < count:
            missing.append(f"{key[0]} ({key[1]}): expected {count}, found {actual.get(key, 0)}")
    if missing:
        raise ValueError(
            f"cannot locate imported references in module '{module['id']}': "
            + "; ".join(missing)
        )
    return found


def validate_generated_imports(modules, classification_by_owner):
    module_by_export = {}
    for module in modules:
        for entry in module["exports"]:
            module_by_export[entry["name"]] = module
    for module in modules:
        for entry in module.get("imports", []):
            target_module = module_by_export.get(entry["target"])
            if target_module is None:
                raise ValueError(
                    f"module '{module['id']}' imports missing export "
                    f"'{entry['target']}'"
                )
            if target_module["owner"] == module["owner"]:
                raise ValueError(
                    f"module '{module['id']}' imports its own export "
                    f"'{entry['target']}'"
                )
            classification = classification_by_owner.get(
                target_module["owner"], "fixed"
            )
            if classification in IMMOVABLE_CLASSIFICATIONS:
                raise ValueError(
                    f"module '{module['id']}' imports fixed module "
                    f"'{target_module['owner']}'"
                )
    return True


def validate_module_isolation(
    module_sources,
    label_owners,
    aliases=None,
    classification_by_owner=None,
):
    violations = []
    aliases = aliases or {}
    for owner, source_text in module_sources.items():
        for target, target_owner, _ in source_module_references(
            owner,
            source_text,
            label_owners,
            aliases,
            classification_by_owner,
            movable_only=False,
            include_addresses=False,
        ):
            if classification_by_owner:
                classification = classification_by_owner.get(
                    target_owner, "fixed"
                )
                if classification in IMMOVABLE_CLASSIFICATIONS:
                    continue
            violations.append(
                f"{owner} references {target} from {target_owner}"
            )
    if violations:
        raise ValueError(
            "direct cross-module reference: " + "; ".join(violations)
        )
    return True


def collect_dispatch_names(header_dir):
    path = header_dir / "20_runtime.asm"
    names = []
    inside = False
    for line in read_text(path).splitlines():
        stripped = line.strip()
        if stripped.startswith("wrapper_routines_map_table:"):
            inside = True
            continue
        if not inside:
            continue
        if stripped.startswith("DEFS"):
            break
        match = re.match(r"^\s*dw\s+([A-Za-z_.][A-Za-z0-9_.]*)\s*(?:;.*)?$", line)
        if match:
            names.append(match.group(1))
    return names


def parse_symbols(path):
    symbols = {}
    for line in read_text(path).splitlines():
        match = SYMBOL_RE.match(line.strip())
        if match:
            symbols[match.group(1)] = int(match.group(2), 16)
    return symbols


def kernel_symbols(source_labels, symbols):
    result = []
    for name, owner in source_labels.items():
        address = symbols.get(name)
        if address is None:
            continue
        if KERNEL_START <= address < KERNEL_END:
            result.append(
                {"name": name, "address": address, "owner": owner}
            )
    result.sort(key=lambda item: (item["address"], item["name"]))
    return result


def build_module_runs(symbols):
    entries = list(symbols)
    if not entries or entries[0]["address"] > KERNEL_START:
        entries.insert(
            0,
            {
                "name": "ROM_HEADER",
                "address": KERNEL_START,
                "owner": entries[0]["owner"] if entries else "20_runtime",
            },
        )

    runs = []
    index = 0
    while index < len(entries):
        address = entries[index]["address"]
        owner = entries[index]["owner"]

        next_index = index
        while (
            next_index < len(entries)
            and entries[next_index]["address"] == address
        ):
            next_index += 1

        end = (
            entries[next_index]["address"]
            if next_index < len(entries)
            else KERNEL_END
        )

        if runs and runs[-1]["owner"] == owner and runs[-1]["end"] == address:
            runs[-1]["end"] = end
        else:
            runs.append(
                {"owner": owner, "start": address, "end": end}
            )

        index = next_index

    total = sum(run["end"] - run["start"] for run in runs)
    if total != KERNEL_END - KERNEL_START:
        raise ValueError(
            "size map does not cover the kernel page: "
            f"{total} bytes instead of {KERNEL_END - KERNEL_START}"
        )

    return runs


def module_exports(symbols, run):
    return [
        item
        for item in symbols
        if item["owner"] == run["owner"]
        and run["start"] <= item["address"] < run["end"]
    ]


def next_symbol(symbols, name):
    address = symbols[name]
    candidates = sorted(
        value for value in symbols.values() if value > address
    )
    if not candidates:
        raise ValueError(f"no symbol follows {name}")
    return candidates[0]


def rounded_block(symbols, names, padding=8):
    addresses = [symbols[name] for name in names]
    return min(addresses), max(addresses) + padding


def build_fixed_regions(symbols, kernel_dir):
    player_bin = kernel_dir / "assets" / "arkos" / "arkosplayer.bin"
    player_size = player_bin.stat().st_size

    player_start = symbols["ARKOS_PLAYER"]
    if player_start != 0x6481:
        raise ValueError(
            "ARKOS_PLAYER moved from its designated address: "
            f"0x{player_start:04X}"
        )

    player_symbols = {
        "PLY_AKM_Init": 0x65AB,
        "PLY_AKM_InitSoundEffects": 0x648A,
        "PLY_AKM_Play": 0x66A8,
        "PLY_AKM_PlaySoundEffect": 0x648E,
        "PLY_AKM_Stop": 0x6692,
        "PLY_AKM_StopSoundEffectFromChannel": 0x64B6,
    }
    for name, expected in player_symbols.items():
        if symbols.get(name) != expected:
            raise ValueError(
                f"{name} moved from its designated address: "
                f"0x{symbols.get(name, 0):04X}"
            )

    interrupt_start = symbols["player.int.control"]
    if interrupt_start != 0x5659:
        raise ValueError(
            "player.int.control moved from its designated address: "
            f"0x{interrupt_start:04X}"
        )
    interrupt_end = symbols[PLAYER_INTERRUPT_NEXT]
    if interrupt_end <= interrupt_start:
        raise ValueError("player interrupt region has an invalid range")

    boot_patch_names = [
        "ascii16x_patch_bugfix_ab_check",
        "konami_patch_bugfix_6800",
        "ascii16_patch_bugfix_inc1",
        "konami_patch_bugfix_8000",
        "ascii16_patch_bugfix_nopseq",
        "konami_patch_bugfix_A000",
    ]
    runtime_patch_names = [
        "konami_patch_sgm_8000",
        "konami_patch_sgm_A000",
        "konami_patch_omsx_0",
        "konami_patch_omsx_1",
        "konami_patch_omsx_2",
        "konami_patch_omsx_3",
        "konami_patch_omsx_4",
    ]

    boot_start, boot_end = rounded_block(symbols, boot_patch_names)
    runtime_start, runtime_end = rounded_block(
        symbols, runtime_patch_names
    )

    trap_names = [
        "C6C25",
        "C6C26",
        "C6C27",
        "XBASIC_TRAP_CHECK",
        "XBASIC_TRAP_ON",
        "XBASIC_TRAP_OFF",
        "XBASIC_TRAP_STOP",
    ]
    trap_start = min(symbols[name] for name in trap_names)
    trap_end = next_symbol(symbols, "XBASIC_TRAP_STOP")

    regions = [
        {
            "id": "rom_header",
            "start": KERNEL_START,
            "end": boot_start,
            "owner": "boot",
        },
        {
            "id": "mapper_patch_boot",
            "start": boot_start,
            "end": boot_end,
            "owner": "boot",
        },
        {
            "id": "player_interrupt",
            "start": interrupt_start,
            "end": interrupt_end,
            "owner": "player",
        },
        {
            "id": "mapper_patch_runtime",
            "start": runtime_start,
            "end": runtime_end,
            "owner": "megarom",
        },
        {
            "id": "arkos_player",
            "start": player_start,
            "end": player_start + player_size,
            "owner": "player",
        },
        {
            "id": "trap_routines",
            "start": trap_start,
            "end": trap_end,
            "owner": "traps",
        },
    ]

    for region in regions:
        if region["start"] >= region["end"]:
            raise ValueError(
                f"fixed region {region['id']} has an invalid range"
            )
        if region["end"] > KERNEL_END:
            raise ValueError(
                f"fixed region {region['id']} exceeds the kernel page"
            )

    ordered = sorted(regions, key=lambda item: item["start"])
    for left, right in zip(ordered, ordered[1:]):
        if left["end"] > right["start"]:
            raise ValueError(
                f"fixed regions {left['id']} and {right['id']} overlap"
            )

    return regions


def read_binary(path):
    return path.read_bytes()


def detect_relocations(module_bytes, internal_addresses):
    offsets = set()
    for offset in range(len(module_bytes) - 1):
        value = module_bytes[offset] | (module_bytes[offset + 1] << 8)
        if value in internal_addresses:
            offsets.add(offset)
    return sorted(offsets)


def detect_dependencies(owner, header_dir, label_owners, aliases):
    source = header_dir / f"{owner}.asm"
    if not source.exists():
        return []
    references = source_module_references(
        owner,
        read_text(source),
        label_owners,
        aliases,
        include_addresses=False,
    )
    return sorted({target_owner for _, target_owner, _ in references})


def detect_patch_points(exports):
    prefixes = (
        "konami_patch_",
        "ascii16_patch_",
        "ascii16x_patch_",
        "megarom_ascii8_bug_fix",
        "OPENMSX_EMULATOR_AUTODETECTION",
    )
    return sorted(
        {
            entry["offset"]
            for entry in exports
            if entry["name"].startswith(prefixes)
        }
    )


def load_module_groups(path):
    if not path.exists():
        return {}
    payload = json.loads(path.read_text(encoding="utf-8"))
    groups = payload.get("groups")
    if not isinstance(groups, dict):
        raise ValueError("module-groups.json must contain a groups object")
    return groups


def classify_modules(modules, groups, groups_path):
    if not groups:
        return
    for module in modules:
        owner = module["owner"]
        if owner not in groups:
            raise ValueError(
                f"{groups_path} has no classification for module {owner}"
            )
        module["group"] = groups[owner]["group"]
        classification = groups[owner]["classification"]
        module["classification"] = classification
        module["policy"] = (
            "fixed" if classification in ("fixed", "pinned") else classification
        )


def build_manifest(kernel_dir, symbols_path):
    header_dir = kernel_dir / "src" / "header"
    symbols = parse_symbols(symbols_path)
    labels = collect_source_labels(header_dir)
    aliases = collect_aliases(header_dir, labels)
    code_symbols = kernel_symbols(labels, symbols)
    runs = build_module_runs(code_symbols)
    fixed_regions = build_fixed_regions(symbols, kernel_dir)

    dispatch_names = collect_dispatch_names(header_dir)
    header_bin = read_binary(kernel_dir / "bin" / "header.bin")
    modules = []
    per_owner = {}
    for run in runs:
        owner = run["owner"]
        per_owner[owner] = per_owner.get(owner, 0) + 1
        module_id = owner if per_owner[owner] == 1 else f"{owner}:{per_owner[owner]}"
        exports = module_exports(code_symbols, run)
        export_records = [
            {"name": item["name"], "offset": item["address"] - run["start"]}
            for item in exports
        ]
        internal_addresses = {
            item["address"] for item in exports
        }
        module_bytes = header_bin[run["start"]:run["end"]]
        modules.append(
            {
                "id": module_id,
                "name": owner,
                "owner": owner,
                "assembly_base": run["start"],
                "size": run["end"] - run["start"],
                "fixed_address": run["start"],
                "has_fixed_address": True,
                "policy": "fixed",
                "exports": export_records,
                "relocations": detect_relocations(
                    module_bytes, internal_addresses
                ),
                "dependencies": detect_dependencies(
                    owner, header_dir, labels, aliases
                ),
                "imports": [],
                "patch_points": detect_patch_points(export_records),
            }
        )

    known_exports = set()
    for module in modules:
        for entry in module["exports"]:
            known_exports.add(entry["name"])

    for name in dispatch_names:
        address = symbols.get(name)
        if address is None or not (KERNEL_START <= address < KERNEL_END):
            continue
        if name in known_exports:
            continue
        for module in modules:
            start = module["assembly_base"]
            end = start + module["size"]
            if start <= address < end:
                module["exports"].append(
                    {"name": name, "offset": address - start}
                )
                known_exports.add(name)
                break

    groups_path = kernel_dir.parent / "module-groups.json"
    classify_modules(modules, load_module_groups(groups_path), groups_path)

    classification_by_owner = {
        module["owner"]: module.get("classification", "fixed")
        for module in modules
    }
    for module in modules:
        source = header_dir / f"{module['owner']}.asm"
        if not source.exists():
            continue
        module["imports"] = detect_module_imports(
            module,
            read_text(source),
            labels,
            aliases,
            classification_by_owner,
            symbols,
            header_bin,
        )
    validate_generated_imports(modules, classification_by_owner)

    return {
        "kernel": {
            "start": KERNEL_START,
            "end": KERNEL_END,
            "size": KERNEL_END - KERNEL_START,
        },
        "modules": modules,
        "fixed_regions": fixed_regions,
        "symbols": code_symbols,
    }


def c_string(value):
    return json.dumps(value)


def write_header(path, manifest):
    lines = []
    lines.append("// Generated by scripts/generate_kernel_manifest.py.")
    lines.append("// Do not edit by hand; run the kernel asm manifest target.")
    lines.append("")
    lines.append("#ifndef KERNEL_MANIFEST_GENERATED_H_INCLUDED")
    lines.append("#define KERNEL_MANIFEST_GENERATED_H_INCLUDED")
    lines.append("")
    lines.append("struct GeneratedKernelExport {")
    lines.append("  const char* name;")
    lines.append("  unsigned int offset;")
    lines.append("};")
    lines.append("")
    lines.append("struct GeneratedKernelImport {")
    lines.append("  const char* target;")
    lines.append("  unsigned int offset;")
    lines.append("  int kind;")
    lines.append("};")
    lines.append("")
    lines.append("struct GeneratedKernelModuleDefinition {")
    lines.append("  const char* id;")
    lines.append("  const char* name;")
    lines.append("  const char* owner;")
    lines.append("  const char* group;")
    lines.append("  const char* classification;")
    lines.append("  unsigned int assembly_base;")
    lines.append("  unsigned int size;")
    lines.append("  unsigned int fixed_address;")
    lines.append("  int has_fixed_address;")
    lines.append("  int policy;")
    lines.append("  const unsigned char* bytes;")
    lines.append("  unsigned int exports_count;")
    lines.append("  const GeneratedKernelExport* exports;")
    lines.append("  unsigned int imports_count;")
    lines.append("  const GeneratedKernelImport* imports;")
    lines.append("  unsigned int relocations_count;")
    lines.append("  const unsigned int* relocations;")
    lines.append("  unsigned int dependencies_count;")
    lines.append("  const char* const* dependencies;")
    lines.append("  unsigned int patch_points_count;")
    lines.append("  const unsigned int* patch_points;")
    lines.append("};")
    lines.append("")
    lines.append("struct GeneratedKernelFixedRegionDefinition {")
    lines.append("  const char* id;")
    lines.append("  unsigned int start;")
    lines.append("  unsigned int end;")
    lines.append("  const char* owner;")
    lines.append("};")
    lines.append("")

    kernel_bytes = read_binary(
        path.parent / "asm" / "bin" / "header.bin"
    )

    for index, module in enumerate(manifest["modules"]):
        start = module["assembly_base"]
        end = start + module["size"]
        lines.append(
            f"static const unsigned char kernel_manifest_bytes_{index}[] = {{"
        )
        chunk = kernel_bytes[start:end]
        for offset in range(0, len(chunk), 16):
            values = ", ".join(
                f"0x{value:02X}" for value in chunk[offset:offset + 16]
            )
            lines.append(f"    {values},")
        lines.append("};")
        lines.append("")

        exports = module["exports"]
        if exports:
            lines.append(
                f"static const GeneratedKernelExport "
                f"kernel_manifest_exports_{index}[] = {{"
            )
            for entry in exports:
                lines.append(
                    f"    {{{c_string(entry['name'])}, {entry['offset']}}},"
                )
            lines.append("};")
            lines.append("")
        else:
            lines.append(
                f"static const GeneratedKernelExport "
                f"kernel_manifest_exports_{index}[] = "
                "{ {0, 0} };"
            )
            lines.append("")

        imports = module.get("imports", [])
        if imports:
            lines.append(
                f"static const GeneratedKernelImport "
                f"kernel_manifest_imports_{index}[] = {{"
            )
            for entry in imports:
                lines.append(
                    f"    {{{c_string(entry['target'])}, "
                    f"{entry['offset']}, {IMPORT_KIND_NUMBERS[entry['kind']]}}},"
                )
            lines.append("};")
            lines.append("")
        else:
            lines.append(
                f"static const GeneratedKernelImport "
                f"kernel_manifest_imports_{index}[] = {{ {{0, 0, 0}} }};"
            )
            lines.append("")

        for field, suffix in (
            ("relocations", "relocations"),
            ("patch_points", "patch_points"),
        ):
            values = module[field]
            if values:
                lines.append(
                    f"static const unsigned int "
                    f"kernel_manifest_{suffix}_{index}[] = {{"
                )
                lines.append(
                    "    " + ", ".join(str(value) for value in values) + ","
                )
                lines.append("};")
                lines.append("")
            else:
                lines.append(
                    f"static const unsigned int "
                    f"kernel_manifest_{suffix}_{index}[] = {{0}};"
                )
                lines.append("")

        dependencies = module["dependencies"]
        if dependencies:
            lines.append(
                f"static const char* const "
                f"kernel_manifest_dependencies_{index}[] = {{"
            )
            for dependency in dependencies:
                lines.append(f"    {c_string(dependency)},")
            lines.append("};")
            lines.append("")
        else:
            lines.append(
                f"static const char* const "
                f"kernel_manifest_dependencies_{index}[] = {{0}};"
            )
            lines.append("")

    lines.append(
        f"#define KERNEL_MANIFEST_MODULE_COUNT {len(manifest['modules'])}"
    )
    lines.append(
        f"#define KERNEL_MANIFEST_FIXED_REGION_COUNT "
        f"{len(manifest['fixed_regions'])}"
    )
    lines.append("")
    lines.append(
        "static const GeneratedKernelModuleDefinition "
        "kernel_manifest_modules[] = {"
    )
    for index, module in enumerate(manifest["modules"]):
        lines.append("    {")
        lines.append(f"        {c_string(module['id'])},")
        lines.append(f"        {c_string(module['name'])},")
        lines.append(f"        {c_string(module['owner'])},")
        lines.append(f"        {c_string(module.get('group', ''))},")
        lines.append(
            f"        {c_string(module.get('classification', 'fixed'))},"
        )
        lines.append(f"        {module['assembly_base']},")
        lines.append(f"        {module['size']},")
        lines.append(f"        {module['fixed_address']},")
        lines.append(f"        {1 if module['has_fixed_address'] else 0},")
        policy_numbers = {
            "fixed": 0,
            "flex-high": 1,
            "flex-low": 2,
            "overflow": 3,
        }
        lines.append(
            f"        {policy_numbers.get(module['policy'], 0)},"
        )
        lines.append(f"        kernel_manifest_bytes_{index},")
        lines.append(f"        {len(module['exports'])},")
        lines.append(f"        kernel_manifest_exports_{index},")
        lines.append(f"        {len(module.get('imports', []))},")
        lines.append(f"        kernel_manifest_imports_{index},")
        lines.append(f"        {len(module['relocations'])},")
        lines.append(f"        kernel_manifest_relocations_{index},")
        lines.append(f"        {len(module['dependencies'])},")
        lines.append(f"        kernel_manifest_dependencies_{index},")
        lines.append(f"        {len(module['patch_points'])},")
        lines.append(f"        kernel_manifest_patch_points_{index},")
        lines.append("    },")
    lines.append("};")
    lines.append("")
    lines.append(
        "static const GeneratedKernelFixedRegionDefinition "
        "kernel_manifest_fixed_regions[] = {"
    )
    for region in manifest["fixed_regions"]:
        lines.append("    {")
        lines.append(f"        {c_string(region['id'])},")
        lines.append(f"        {region['start']},")
        lines.append(f"        {region['end']},")
        lines.append(f"        {c_string(region['owner'])},")
        lines.append("    },")
    lines.append("};")
    lines.append("")
    lines.append("#endif  // KERNEL_MANIFEST_GENERATED_H_INCLUDED")
    lines.append("")

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def write_dispatch_header(path, names):
    lines = []
    lines.append("// Generated by scripts/generate_kernel_manifest.py.")
    lines.append("// Do not edit by hand; run the kernel asm manifest target.")
    lines.append("")
    lines.append("#ifndef KERNEL_DISPATCH_GENERATED_H_INCLUDED")
    lines.append("#define KERNEL_DISPATCH_GENERATED_H_INCLUDED")
    lines.append("")
    lines.append("#define KERNEL_DISPATCH_COUNT %d" % len(names))
    lines.append("")
    lines.append("static const char* kernel_dispatch_names[] = {")
    for name in names:
        lines.append("    %s," % c_string(name))
    lines.append("};")
    lines.append("")
    lines.append("#endif  // KERNEL_DISPATCH_GENERATED_H_INCLUDED")
    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def write_markdown(path, manifest):
    lines = []
    lines.append("# Kernel Baseline Size Map")
    lines.append("")
    lines.append(
        "Generated from `header.bin`, `header.symbols.asm`, and the "
        "header assembly sources."
    )
    lines.append("")
    lines.append(
        f"Kernel page: `0x{manifest['kernel']['start']:04X}` to "
        f"`0x{manifest['kernel']['end']:04X}` "
        f"({manifest['kernel']['size']} bytes)."
    )
    lines.append("")
    lines.append("## Modules")
    lines.append("")
    lines.append("| Module | Start | End | Size | Owner |")
    lines.append("| --- | ---: | ---: | ---: | --- |")
    for module in manifest["modules"]:
        lines.append(
            f"| `{module['id']}` | `0x{module['assembly_base']:04X}` | "
            f"`0x{module['assembly_base'] + module['size']:04X}` | "
            f"{module['size']} | `{module['owner']}` |"
        )
    lines.append("")
    lines.append("## Fixed Regions")
    lines.append("")
    lines.append("| Region | Start | End | Size | Owner |")
    lines.append("| --- | ---: | ---: | ---: | --- |")
    for region in manifest["fixed_regions"]:
        lines.append(
            f"| `{region['id']}` | `0x{region['start']:04X}` | "
            f"`0x{region['end']:04X}` | "
            f"{region['end'] - region['start']} | `{region['owner']}` |"
        )
    lines.append("")
    lines.append("## Routines")
    lines.append("")
    lines.append("| Routine | Owner | Start | End | Size |")
    lines.append("| --- | --- | ---: | ---: | ---: |")
    symbols = manifest["symbols"]
    for index, symbol in enumerate(symbols):
        end = (
            symbols[index + 1]["address"]
            if index + 1 < len(symbols)
            else KERNEL_END
        )
        if end <= symbol["address"]:
            continue
        lines.append(
            f"| `{symbol['name']}` | `{symbol['owner']}` | "
            f"`0x{symbol['address']:04X}` | `0x{end:04X}` | "
            f"{end - symbol['address']} |"
        )
    lines.append("")

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parent.parent
    parser.add_argument(
        "--kernel-dir",
        type=Path,
        default=root / "src" / "infrastructure" / "kernel" / "asm",
    )
    parser.add_argument(
        "--symbols",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "asm"
        / "src"
        / "header.symbols.asm",
    )
    parser.add_argument(
        "--out-header",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "kernel_manifest_generated.h",
    )
    parser.add_argument(
        "--out-dispatch-header",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "kernel_dispatch_generated.h",
    )
    parser.add_argument(
        "--out-json",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "asm"
        / "bin"
        / "baseline-manifest.json",
    )
    parser.add_argument(
        "--out-markdown",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "asm"
        / "bin"
        / "baseline-size-map.md",
    )
    args = parser.parse_args(argv)

    manifest = build_manifest(args.kernel_dir, args.symbols)

    payload = dict(manifest)
    payload.pop("symbols")
    args.out_json.parent.mkdir(parents=True, exist_ok=True)
    args.out_json.write_text(
        json.dumps(payload, indent=2) + "\n", encoding="utf-8"
    )
    write_header(args.out_header, manifest)
    write_dispatch_header(
        args.out_dispatch_header,
        collect_dispatch_names(args.kernel_dir / "src" / "header"),
    )
    args.out_markdown.parent.mkdir(parents=True, exist_ok=True)
    write_markdown(args.out_markdown, manifest)

    print(
        f"Generated {len(manifest['modules'])} modules, "
        f"{len(manifest['fixed_regions'])} fixed regions, "
        f"{len(manifest['symbols'])} routines."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
