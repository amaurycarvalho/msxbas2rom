#!/usr/bin/env python3
"""Generate relocatable module binaries and relocation records.

Each configured module is assembled twice at different bases.  The dual-base
diff identifies internal absolute references, and the generated header is
consumed by the generated kernel module library.  This runs at project build
time; BASIC compilation never invokes pasmo.
"""

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

KERNEL_START = 0x4000
KERNEL_END = 0x8000
BASE_A = 0x0000
BASE_B = 0x0100
DELTA = BASE_B - BASE_A

LABEL_RE = re.compile(r"^\s*([A-Za-z_.][\w.]*)\s*:\s*(\S*)")
EQU_RE = re.compile(
    r"^(\s*)([A-Za-z_.$][\w.$]*)(\s*:?\s*equ\s+)([A-Za-z_.$][\w.$]*)(.*)$",
    re.IGNORECASE,
)
SYMBOL_RE = re.compile(
    r"^([A-Za-z_.$][\w.$]*)\s+EQU\s+([0-9A-Fa-f]+)H", re.IGNORECASE
)


def read_text(path):
    return path.read_text(encoding="utf-8", errors="replace")


def parse_symbols(path):
    symbols = {}
    for line in read_text(path).splitlines():
        match = SYMBOL_RE.match(line.strip())
        if match:
            symbols[match.group(1)] = int(match.group(2), 16)
    return symbols


def parse_labels(path, owner):
    labels = {}
    for line in read_text(path).splitlines():
        match = LABEL_RE.match(line)
        if not match:
            continue
        if match.group(2).lower() in ("equ", "="):
            continue
        labels[match.group(1)] = owner
    return labels


def collect_defined_names(path):
    names = {}
    owner = path.stem
    for line in read_text(path).splitlines():
        label = LABEL_RE.match(line)
        if label and label.group(2).lower() not in ("equ", "="):
            names[label.group(1)] = owner
            continue
        equ = re.match(
            r"^\s*([A-Za-z_.$][\w.$]*)\s*:?\s*equ\b", line,
            re.IGNORECASE,
        )
        if equ:
            names[equ.group(1)] = owner
    return names


def collect_all_labels(header_dir):
    labels = {}
    for path in sorted(header_dir.glob("*.asm")):
        labels.update(parse_labels(path, path.stem))
    return labels


def defined_equ_names(path):
    names = set()
    for line in read_text(path).splitlines():
        match = re.match(
            r"^\s*([A-Za-z_.$][\w.$]*)\s*:?\s*equ\b", line, re.IGNORECASE
        )
        if match:
            names.add(match.group(1))
    return names


def make_constants(header_dir, symbols, output):
    source = header_dir / "00_constants_and_workarea.asm"
    lines = []
    for line in read_text(source).splitlines():
        match = EQU_RE.match(line)
        if match and match.group(4) in symbols:
            line = (
                f"{match.group(1)}{match.group(2)}{match.group(3)}"
                f"0x{symbols[match.group(4)]:04X}{match.group(5)}"
            )
        lines.append(line)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def build_wrapper(header_dir, kernel_dir, symbols, all_defined, module,
                  base, wrapper_path, constants_name):
    owner = module["owner"]
    source = module.get("source", f"{owner}.asm")
    target_names = set(collect_defined_names(header_dir / source).keys())
    constants_path = header_dir / constants_name
    constant_names = set(collect_defined_names(constants_path).keys())

    fixed_owners = module.get("_fixed_owners", set())

    lines = [f'include "src/header/{constants_name}"',
             'include "src/header/10_macros.asm"', "",
             "; fixed core symbols"]
    for name, name_owner in sorted(all_defined.items()):
        if name_owner not in fixed_owners:
            continue
        if name in target_names or name in constant_names:
            continue
        if name not in symbols:
            continue
        lines.append(f"{name} EQU 0x{symbols[name]:04X}")
    lines += ["", f"org 0x{base:04X}",
              f'include "src/header/{source}"', ""]
    wrapper_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def assemble(kernel_dir, wrapper_name, output_name, symbol_name):
    result = subprocess.run(
        ["pasmo", wrapper_name, output_name, symbol_name],
        cwd=kernel_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"pasmo failed for {wrapper_name}:\n{result.stdout}\n{result.stderr}"
        )
    return ((kernel_dir / output_name).read_bytes(),
            parse_symbols(kernel_dir / symbol_name))


def detect_relocations(bytes_a, bytes_b, label_addresses):
    offsets = []
    for offset in range(len(bytes_a) - 1):
        value_a = bytes_a[offset] | (bytes_a[offset + 1] << 8)
        value_b = bytes_b[offset] | (bytes_b[offset + 1] << 8)
        if value_a == value_b:
            continue
        if value_a not in label_addresses:
            continue
        if ((value_b - value_a) & 0xFFFF) != DELTA:
            continue
        offsets.append(offset)
    return offsets


IMPORT_KIND_NUMBERS = {
    "call": 0,
    "jump": 1,
    "conditional": 2,
    "address": 3,
}


def c_string(value):
    return json.dumps(value)


def write_header(path, modules):
    lines = [
        "// Generated by scripts/generate_relocatable_modules.py.",
        "// Do not edit by hand; run the kernel asm manifest target.",
        "",
        "#ifndef KERNEL_RELOCATABLE_GENERATED_H_INCLUDED",
        "#define KERNEL_RELOCATABLE_GENERATED_H_INCLUDED",
        "",
        "struct GeneratedRelocatableExport {",
        "  const char* name;",
        "  unsigned int offset;",
        "};",
        "",
        "struct GeneratedRelocatableImport {",
        "  const char* target;",
        "  unsigned int offset;",
        "  int kind;",
        "};",
        "",
        "struct GeneratedRelocatableModuleDefinition {",
        "  const char* id;",
        "  const char* owner;",
        "  const char* group;",
        "  const char* classification;",
        "  int policy;",
        "  int force_overflow;",
        "  const unsigned char* bytes;",
        "  unsigned int size;",
        "  unsigned int exports_count;",
        "  const GeneratedRelocatableExport* exports;",
        "  unsigned int imports_count;",
        "  const GeneratedRelocatableImport* imports;",
        "  unsigned int relocations_count;",
        "  const unsigned int* relocations;",
        "  unsigned int dependencies_count;",
        "  const char* const* dependencies;",
        "};",
        "",
    ]

    for index, module in enumerate(modules):
        lines.append(
            f"static const unsigned char kernel_relocatable_bytes_{index}[] = {{"
        )
        for offset in range(0, len(module["bytes"]), 16):
            values = ", ".join(
                f"0x{value:02X}"
                for value in module["bytes"][offset:offset + 16]
            )
            lines.append(f"    {values},")
        lines.append("};")
        lines.append("")

        if module["exports"]:
            lines.append(
                f"static const GeneratedRelocatableExport "
                f"kernel_relocatable_exports_{index}[] = {{"
            )
            for entry in module["exports"]:
                lines.append(
                    f"    {{{c_string(entry['name'])}, {entry['offset']}}},"
                )
            lines.append("};")
        else:
            lines.append(
                f"static const GeneratedRelocatableExport "
                f"kernel_relocatable_exports_{index}[] = {{ {{0, 0}} }};"
            )
        lines.append("")

        imports = module.get("imports", [])
        if imports:
            lines.append(
                f"static const GeneratedRelocatableImport "
                f"kernel_relocatable_imports_{index}[] = {{"
            )
            for entry in imports:
                lines.append(
                    f"    {{{c_string(entry['target'])}, "
                    f"{entry['offset']}, {IMPORT_KIND_NUMBERS[entry['kind']]}}},"
                )
            lines.append("};")
        else:
            lines.append(
                f"static const GeneratedRelocatableImport "
                f"kernel_relocatable_imports_{index}[] = {{ {{0, 0, 0}} }};"
            )
        lines.append("")

        if module["relocations"]:
            lines.append(
                f"static const unsigned int "
                f"kernel_relocatable_relocations_{index}[] = {{"
            )
            lines.append(
                "    "
                + ", ".join(str(value) for value in module["relocations"])
                + ","
            )
            lines.append("};")
        else:
            lines.append(
                f"static const unsigned int "
                f"kernel_relocatable_relocations_{index}[] = {{0}};"
            )
        lines.append("")

        if module["dependencies"]:
            lines.append(
                f"static const char* const "
                f"kernel_relocatable_dependencies_{index}[] = {{"
            )
            for dependency in module["dependencies"]:
                lines.append(f"    {c_string(dependency)},")
            lines.append("};")
        else:
            lines.append(
                f"static const char* const "
                f"kernel_relocatable_dependencies_{index}[] = {{0}};"
            )
        lines.append("")

    policy_numbers = {
        "fixed": 0,
        "flex-high": 1,
        "flex-low": 2,
        "overflow": 3,
    }
    lines.append(
        f"#define KERNEL_RELOCATABLE_MODULE_COUNT {len(modules)}"
    )
    lines.append("")
    lines.append(
        "static const GeneratedRelocatableModuleDefinition "
        "kernel_relocatable_modules[] = {"
    )
    for index, module in enumerate(modules):
        lines.append("    {")
        lines.append(f"        {c_string(module['id'])},")
        lines.append(f"        {c_string(module['owner'])},")
        lines.append(f"        {c_string(module['group'])},")
        lines.append(f"        {c_string(module['classification'])},")
        lines.append(
            f"        {policy_numbers.get(module['policy'], 1)},"
        )
        lines.append(
            f"        {1 if module.get('force_overflow') else 0},"
        )
        lines.append(f"        kernel_relocatable_bytes_{index},")
        lines.append(f"        {len(module['bytes'])},")
        lines.append(f"        {len(module['exports'])},")
        lines.append(f"        kernel_relocatable_exports_{index},")
        lines.append(f"        {len(module.get('imports', []))},")
        lines.append(f"        kernel_relocatable_imports_{index},")
        lines.append(f"        {len(module['relocations'])},")
        lines.append(f"        kernel_relocatable_relocations_{index},")
        lines.append(f"        {len(module['dependencies'])},")
        lines.append(f"        kernel_relocatable_dependencies_{index},")
        lines.append("    },")
    lines.append("};")
    lines.append("")
    lines.append("#endif  // KERNEL_RELOCATABLE_GENERATED_H_INCLUDED")
    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def main(argv=None):
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--kernel-dir",
        type=Path,
        default=root / "src" / "infrastructure" / "kernel" / "asm",
    )
    parser.add_argument(
        "--manifest",
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
        "--config",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "relocatable-modules.json",
    )
    parser.add_argument(
        "--out-header",
        type=Path,
        default=root
        / "src"
        / "infrastructure"
        / "kernel"
        / "kernel_relocatable_generated.h",
    )
    args = parser.parse_args(argv)

    header_dir = args.kernel_dir / "src" / "header"
    symbols = parse_symbols(args.kernel_dir / "src" / "header.symbols.asm")
    all_labels = collect_all_labels(header_dir)
    all_defined = {}
    for path in sorted(header_dir.glob("*.asm")):
        all_defined.update(collect_defined_names(path))
    manifest = json.loads(read_text(args.manifest))
    config = json.loads(read_text(args.config))

    modules_by_id = {module["id"]: module for module in manifest["modules"]}
    fixed_owners = {
        module["owner"]
        for module in manifest["modules"]
        if module["has_fixed_address"]
    }

    results = []
    with tempfile.TemporaryDirectory(dir=args.kernel_dir) as directory:
        workdir = Path(directory)
        constants_name = f"{workdir.name}_constants.asm"
        constants_path = header_dir / constants_name
        make_constants(header_dir, symbols, constants_path)
        try:
            def process(module, force_overflow):
                module = dict(module)
                module["_fixed_owners"] = fixed_owners
                source = module.get("source", f"{module['owner']}.asm")
                module["source"] = source

                wrapper_a = workdir / f"wrapper_{module['id']}_a.asm"
                wrapper_b = workdir / f"wrapper_{module['id']}_b.asm"
                build_wrapper(header_dir, args.kernel_dir, symbols,
                              all_defined, module, BASE_A, wrapper_a,
                              constants_name)
                build_wrapper(header_dir, args.kernel_dir, symbols,
                              all_defined, module, BASE_B, wrapper_b,
                              constants_name)

                out_a = workdir / f"out_{module['id']}_a.bin"
                out_b = workdir / f"out_{module['id']}_b.bin"
                sym_a = workdir / f"out_{module['id']}_a.sym"
                sym_b = workdir / f"out_{module['id']}_b.sym"
                bytes_a, symbols_a = assemble(
                    args.kernel_dir,
                    str(wrapper_a.relative_to(args.kernel_dir)),
                    str(out_a.relative_to(args.kernel_dir)),
                    str(sym_a.relative_to(args.kernel_dir)),
                )
                bytes_b, _ = assemble(
                    args.kernel_dir,
                    str(wrapper_b.relative_to(args.kernel_dir)),
                    str(out_b.relative_to(args.kernel_dir)),
                    str(sym_b.relative_to(args.kernel_dir)),
                )

                target_labels = set(
                    parse_labels(header_dir / source,
                                 module["owner"]).keys()
                )
                label_addresses = {
                    symbols_a[name]
                    for name in target_labels
                    if name in symbols_a and 0 <= symbols_a[name] < len(bytes_a)
                }
                exports = [
                    {"name": name, "offset": symbols_a[name]}
                    for name in sorted(target_labels)
                    if name in symbols_a
                    and 0 <= symbols_a[name] < len(bytes_a)
                ]
                exports_by_name = {entry["name"]: entry for entry in exports}
                for entry in module.get("exports", []):
                    exports_by_name.setdefault(entry["name"], entry)
                exports = sorted(
                    exports_by_name.values(),
                    key=lambda entry: (entry["offset"], entry["name"]),
                )
                relocations = detect_relocations(
                    bytes_a, bytes_b, label_addresses
                )

                module["bytes"] = list(bytes_a)
                module["exports"] = exports
                module["relocations"] = relocations
                module["policy"] = module.get("policy", "flex-high")
                if module["policy"] == "fixed":
                    module["policy"] = "flex-high"
                module["force_overflow"] = force_overflow
                results.append(module)

            for entry in config.get("modules", []):
                if isinstance(entry, str):
                    module_id = entry
                    force_overflow = False
                else:
                    module_id = entry["id"]
                    force_overflow = bool(entry.get("force_overflow", False))
                if module_id not in modules_by_id:
                    raise ValueError(f"unknown relocatable module {module_id}")
                process(modules_by_id[module_id], force_overflow)

            for entry in config.get("extra_modules", []):
                module = dict(entry)
                module.setdefault("owner", module["id"])
                process(module, bool(module.get("force_overflow", False)))
        finally:
            if constants_path.exists():
                constants_path.unlink()

    write_header(args.out_header, results)
    print(
        f"Generated {len(results)} relocatable modules "
        f"({', '.join(module['id'] for module in results)})."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
