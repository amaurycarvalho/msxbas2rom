#!/usr/bin/env python3
"""Enforce the project's inward dependency rule.

The checker scans domain and application sources for layer violations and for
direct filesystem, generated-kernel, concrete-logger, and concrete-filesystem
access.  Violations are only accepted when an exact entry in the versioned
exception list matches them.  Exceptions are validated in both directions:
new violations fail the check, and stale exceptions fail it too.
"""

import argparse
import json
import re
import sys
from pathlib import Path

LAYERS = ("domain", "application", "cli", "infrastructure")
HEADER_SUFFIXES = (".h", ".hpp")
SOURCE_SUFFIXES = (".c", ".cc", ".cpp", ".h", ".hpp")

IO_FUNCTION_PATTERN = re.compile(r"\b(fopen|remove)\s*\(")
IO_TYPE_PATTERN = re.compile(r"\b(ifstream|ofstream)\b")
KERNEL_PATTERN = re.compile(r"\bbin_header_bin\b")
GLOBAL_PROVIDER_PATTERN = re.compile(r"\b(?:get|set)Default[A-Za-z0-9_]*\b")

# Headers that represent concrete outer details.  Application and domain code
# must depend on its own ports instead of on these headers.
CONCRETE_HEADERS = {
    "fswrapper.h",
    "pletter.h",
    "header.h",
    "start.h",
}


def strip_comments(text: str, keep_strings: bool = False) -> str:
    """Return source text without comments, optionally without strings."""
    out = []
    i = 0
    n = len(text)
    state = "code"
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if state == "code":
            if c == "/" and nxt == "/":
                state = "line_comment"
                out.extend("  ")
                i += 2
                continue
            if c == "/" and nxt == "*":
                state = "block_comment"
                out.extend("  ")
                i += 2
                continue
            if not keep_strings and c == '"':
                state = "string"
                out.append(" ")
                i += 1
                continue
            if not keep_strings and c == "'":
                state = "char"
                out.append(" ")
                i += 1
                continue
            out.append(c)
            i += 1
        elif state == "line_comment":
            if c == "\n":
                state = "code"
                out.append("\n")
            else:
                out.append(" ")
            i += 1
        elif state == "block_comment":
            if c == "*" and nxt == "/":
                state = "code"
                out.extend("  ")
                i += 2
            else:
                out.append("\n" if c == "\n" else " ")
                i += 1
        elif state in ("string", "char"):
            quote = '"' if state == "string" else "'"
            if c == "\\" and i + 1 < n:
                out.extend("  ")
                i += 2
                continue
            if c == quote:
                state = "code"
            out.append("\n" if c == "\n" else " ")
            i += 1
    return "".join(out)


def header_layers(root: Path):
    mapping = {}
    src = root / "src"
    if not src.exists():
        return mapping
    for path in src.rglob("*"):
        if path.suffix not in HEADER_SUFFIXES or not path.is_file():
            continue
        rel = path.relative_to(src)
        layer = rel.parts[0] if rel.parts else ""
        mapping.setdefault(path.name, set()).add(layer)
    return mapping


def includer_layer(rel: Path) -> str:
    parts = rel.parts
    if parts and parts[0] == "src":
        parts = parts[1:]
    return parts[0] if parts else ""


def scan_file(root: Path, path: Path, headers):
    rel = path.relative_to(root)
    layer = includer_layer(rel)
    if layer not in ("domain", "application"):
        return []

    raw = path.read_text(encoding="utf-8", errors="replace")
    without_comments = strip_comments(raw, keep_strings=True)
    active = strip_comments(raw, keep_strings=False)
    violations = []

    # Layer direction, based only on local project headers.
    for match in re.finditer(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', without_comments, re.M):
        include = match.group(1)
        header = Path(include).name
        layers = headers.get(header, set())
        forbidden_layer = False
        if layer == "domain":
            forbidden_layer = bool(layers & {"application", "cli", "infrastructure"})
        elif layer == "application":
            forbidden_layer = "cli" in layers
        if forbidden_layer:
            violations.append(
                {
                    "kind": "include",
                    "pattern": header,
                    "message": "forbidden include '%s'" % include,
                    "line": without_comments[: match.start()].count("\n") + 1,
                }
            )
        elif header in CONCRETE_HEADERS:
            violations.append(
                {
                    "kind": "include",
                    "pattern": header,
                    "message": "forbidden concrete dependency '%s'" % include,
                    "line": without_comments[: match.start()].count("\n") + 1,
                }
            )

    for pattern in (IO_FUNCTION_PATTERN, IO_TYPE_PATTERN):
        for match in pattern.finditer(active):
            violations.append(
                {
                    "kind": "pattern",
                    "pattern": match.group(1),
                    "message": "direct filesystem access '%s'" % match.group(1),
                    "line": active[: match.start()].count("\n") + 1,
                }
            )

    for match in KERNEL_PATTERN.finditer(active):
        violations.append(
            {
                "kind": "pattern",
                "pattern": "bin_header_bin",
                "message": "direct generated-kernel access 'bin_header_bin'",
                "line": active[: match.start()].count("\n") + 1,
            }
        )

    # De-duplicate per path/kind/pattern; keep the first line.
    unique = {}
    for item in violations:
        key = (item["kind"], item["pattern"])
        unique.setdefault(key, item)
    return list(unique.values())


def load_exceptions(path: Path):
    if not path.exists():
        raise ValueError("exception file not found: %s" % path)
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ValueError("malformed exception file %s: %s" % (path, exc))
    if not isinstance(payload, dict):
        raise ValueError("malformed exception file: root must be an object")
    exceptions = payload.get("exceptions")
    if not isinstance(exceptions, list):
        raise ValueError("malformed exception file: 'exceptions' must be a list")
    result = []
    for index, entry in enumerate(exceptions):
        if not isinstance(entry, dict):
            raise ValueError("malformed exception entry %d: must be an object" % index)
        paths = entry.get("paths")
        if not isinstance(paths, list) or not paths or not all(
            isinstance(p, str) and p for p in paths
        ):
            raise ValueError(
                "malformed exception entry %d: 'paths' must be a non-empty string list"
                % index
            )
        patterns = entry.get("patterns")
        if not isinstance(patterns, list) or not patterns or not all(
            isinstance(p, str) and p for p in patterns
        ):
            raise ValueError(
                "malformed exception entry %d: 'patterns' must be a non-empty string list"
                % index
            )
        kind = entry.get("kind", "pattern")
        if kind not in ("pattern", "include"):
            raise ValueError(
                "malformed exception entry %d: 'kind' must be 'pattern' or 'include'"
                % index
            )
        for field in ("id", "owner", "removal"):
            if not isinstance(entry.get(field), str) or not entry.get(field):
                raise ValueError(
                    "malformed exception entry %d: '%s' is required" % (index, field)
                )
        result.append(
            {
                "id": entry["id"],
                "paths": paths,
                "patterns": patterns,
                "kind": kind,
                "owner": entry["owner"],
                "removal": entry["removal"],
            }
        )
    return result


def scan_global_providers(root: Path):
    """Reject process-wide composition providers anywhere under src/."""
    errors = []
    src = root / "src"
    if not src.exists():
        return errors
    for path in sorted(src.rglob("*")):
        if path.suffix not in SOURCE_SUFFIXES or not path.is_file():
            continue
        raw = path.read_text(encoding="utf-8", errors="replace")
        active = strip_comments(raw, keep_strings=False)
        for match in GLOBAL_PROVIDER_PATTERN.finditer(active):
            line = active[: match.start()].count("\n") + 1
            rel = str(path.relative_to(root))
            errors.append(
                "%s:%d: global composition provider '%s' is not permitted"
                % (rel, line, match.group(0))
            )
    return errors


def scan_test_composition(root: Path):
    """Reject test-wide adapter installation in the unit test entry point."""
    errors = []
    test_main = root / "tests" / "unit" / "src" / "test_main.cpp"
    if not test_main.exists():
        return errors
    raw = test_main.read_text(encoding="utf-8", errors="replace")
    active = strip_comments(raw, keep_strings=False)
    forbidden = ("setDefault", "DefaultAdaptersSetup", "TestSourceResolver",
                 "FileSourceResolver", "FileRomWriter",
                 "FileResourceContentSource", "FileSymbolOutputSink")
    for token in forbidden:
        if token in active:
            errors.append(
                "%s: test-wide adapter setup '%s' is not permitted"
                % (test_main.relative_to(root), token)
            )
    return errors


def check(root: Path, exceptions_path: Path):
    headers = header_layers(root)
    violations = []
    for layer in ("domain", "application"):
        base = root / "src" / layer
        if not base.exists():
            continue
        for path in sorted(base.rglob("*")):
            if path.suffix in SOURCE_SUFFIXES and path.is_file():
                for item in scan_file(root, path, headers):
                    item["path"] = str(path.relative_to(root))
                    violations.append(item)

    errors = []
    errors.extend(scan_global_providers(root))
    errors.extend(scan_test_composition(root))
    try:
        exceptions = load_exceptions(exceptions_path)
    except ValueError as exc:
        return [], [], [str(exc)]

    matched = set()
    for violation in violations:
        approved = False
        for index, entry in enumerate(exceptions):
            if entry["kind"] != violation["kind"]:
                continue
            if violation["path"] not in entry["paths"]:
                continue
            if violation["pattern"] not in entry["patterns"]:
                continue
            approved = True
            matched.add((index, violation["path"]))
        if not approved:
            errors.append(
                "%s:%d: %s"
                % (violation["path"], violation["line"], violation["message"])
            )

    for index, entry in enumerate(exceptions):
        for path in entry["paths"]:
            if not (root / path).exists():
                errors.append(
                    "exception %s: path does not exist: %s" % (entry["id"], path)
                )
                continue
            if (index, path) not in matched:
                errors.append(
                    "exception %s: stale entry for %s (%s)"
                    % (entry["id"], path, ", ".join(entry["patterns"]))
                )
    return violations, exceptions, errors


def main(argv=None):
    default_root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=default_root)
    parser.add_argument(
        "--exceptions",
        type=Path,
        default=None,
        help="exception JSON (default: <root>/scripts/architecture-exceptions.json)",
    )
    args = parser.parse_args(argv)

    root = args.root.resolve()
    exceptions_path = args.exceptions or (root / "scripts" / "architecture-exceptions.json")
    violations, exceptions, errors = check(root, exceptions_path)

    if errors:
        print("Clean architecture check failed:")
        for error in errors:
            print("  - " + error)
        return 1

    print(
        "Clean architecture check passed: %d approved violation(s) across %d exception(s)."
        % (len(violations), len(exceptions))
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
