#!/usr/bin/env python3
"""Fixture-based tests for check_clean_architecture.py."""

import json
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(SCRIPTS))

import check_clean_architecture as checker  # noqa: E402


def write(root: Path, relative: str, content: str) -> Path:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    return path


def write_exceptions(root: Path, exceptions):
    payload = {"version": 1, "exceptions": exceptions}
    return write(
        root,
        "scripts/architecture-exceptions.json",
        json.dumps(payload, indent=2),
    )


def exception(path, patterns, kind="pattern", **kwargs):
    entry = {
        "id": kwargs.get("id", "X1"),
        "kind": kind,
        "paths": [path],
        "patterns": patterns,
        "owner": kwargs.get("owner", "test"),
        "removal": kwargs.get("removal", "test removal"),
    }
    return entry


class CheckerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        write(self.root, "src/application/app_header.h", "#pragma once\n")
        write(self.root, "src/cli/cli_header.h", "#pragma once\n")
        write(self.root, "src/infrastructure/infra_header.h", "#pragma once\n")
        self.exceptions = write_exceptions(self.root, [])

    def tearDown(self):
        self.tmp.cleanup()

    def run_check(self):
        return checker.check(self.root, self.exceptions)

    def test_domain_include_of_application_fails(self):
        write(
            self.root,
            "src/domain/bad.cpp",
            '#include "app_header.h"\n',
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("forbidden include 'app_header.h'" in error for error in errors),
            errors,
        )

    def test_application_include_of_cli_fails(self):
        write(
            self.root,
            "src/application/bad.cpp",
            '#include "cli_header.h"\n',
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("forbidden include 'cli_header.h'" in error for error in errors),
            errors,
        )

    def test_direct_filesystem_access_fails(self):
        write(
            self.root,
            "src/application/bad.cpp",
            '#include <cstdio>\nint f() { return fopen("x", "r") != 0; }\n',
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("direct filesystem access 'fopen'" in error for error in errors),
            errors,
        )

    def test_ifstream_and_ofstream_types_fail(self):
        write(
            self.root,
            "src/application/bad.cpp",
            "#include <fstream>\n"
            "void f() { std::ifstream in(\"x\"); std::ofstream out(\"y\"); }\n",
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("direct filesystem access 'ifstream'" in error for error in errors),
            errors,
        )
        self.assertTrue(
            any("direct filesystem access 'ofstream'" in error for error in errors),
            errors,
        )

    def test_comments_are_ignored(self):
        write(
            self.root,
            "src/application/ok.cpp",
            "// fopen(\"x\", \"r\")\n"
            "/* ifstream in(\"x\"); ofstream out(\"y\"); bin_header_bin */\n"
            "int f() { return 0; }\n",
        )
        violations, _, errors = self.run_check()
        self.assertEqual(errors, [])
        self.assertEqual(violations, [])

    def test_approved_exception_passes(self):
        self.exceptions = write_exceptions(
            self.root,
            [exception("src/application/approved.cpp", ["fopen"])],
        )
        write(
            self.root,
            "src/application/approved.cpp",
            '#include <cstdio>\nint f() { return fopen("x", "r") != 0; }\n',
        )
        violations, _, errors = self.run_check()
        self.assertEqual(errors, [])
        self.assertEqual(len(violations), 1)

    def test_stale_exception_fails(self):
        self.exceptions = write_exceptions(
            self.root,
            [exception("src/application/clean.cpp", ["fopen"])],
        )
        write(self.root, "src/application/clean.cpp", "int f() { return 0; }\n")
        _, _, errors = self.run_check()
        self.assertTrue(
            any("stale entry for src/application/clean.cpp" in error for error in errors),
            errors,
        )

    def test_missing_exception_path_fails(self):
        self.exceptions = write_exceptions(
            self.root,
            [exception("src/application/missing.cpp", ["fopen"])],
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("path does not exist" in error for error in errors), errors
        )

    def test_malformed_exception_file_fails(self):
        self.exceptions = write(self.root, "scripts/bad.json", "{not json")
        _, _, errors = self.run_check()
        self.assertTrue(any("malformed exception file" in error for error in errors), errors)

    def test_kernel_global_access_fails(self):
        write(
            self.root,
            "src/application/kernel_user.cpp",
            "unsigned char read_byte(unsigned char* data) { return data[0]; }\n"
            "unsigned char g() { return bin_header_bin[0]; }\n",
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("direct generated-kernel access" in error for error in errors), errors
        )


    def test_global_provider_in_src_fails(self):
        write(
            self.root,
            "src/application/provider.cpp",
            "void setDefaultThing();\n",
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("global composition provider 'setDefaultThing'" in error
                for error in errors),
            errors,
        )

    def test_get_default_provider_in_cli_fails(self):
        write(
            self.root,
            "src/cli/provider.cpp",
            "int getDefaultThing() { return 0; }\n",
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("global composition provider 'getDefaultThing'" in error
                for error in errors),
            errors,
        )

    def test_test_main_adapter_setup_fails(self):
        write(
            self.root,
            "tests/unit/src/test_main.cpp",
            "void install() { setDefaultSourceResolver(nullptr); }\n",
        )
        _, _, errors = self.run_check()
        self.assertTrue(
            any("test-wide adapter setup 'setDefault'" in error
                for error in errors),
            errors,
        )

    def test_test_main_without_setup_passes(self):
        write(
            self.root,
            "tests/unit/src/test_main.cpp",
            "#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN\n",
        )
        _, _, errors = self.run_check()
        self.assertEqual(errors, [])

    def test_real_tree_passes(self):
        real_root = SCRIPTS.parent
        real_exceptions = real_root / "scripts" / "architecture-exceptions.json"
        _, _, errors = checker.check(real_root, real_exceptions)
        self.assertEqual(errors, [])


if __name__ == "__main__":
    unittest.main()
