#!/usr/bin/env python3
"""Fixture-based tests for check_test_inventory.py."""

import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

SCRIPTS = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(SCRIPTS))

import check_test_inventory as inventory  # noqa: E402


def write(root: Path, relative: str, content: str) -> Path:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    return path


class TestInventoryTests(unittest.TestCase):
    def setUp(self):
        self.tmp = TemporaryDirectory()
        self.root = Path(self.tmp.name)
        (self.root / "tests" / "unit" / "src").mkdir(parents=True)

    def tearDown(self):
        self.tmp.cleanup()

    def test_ofstream_bas_fails(self):
        write(
            self.root,
            "tests/unit/src/test_compile.cpp",
            'void f() { std::ofstream out("tmp/program.bas"); }\n',
        )
        errors = inventory.check(self.root)
        self.assertTrue(any("filesystem access" in error for error in errors), errors)

    def test_remove_bas_fails(self):
        write(
            self.root,
            "tests/unit/src/test_compile.cpp",
            'void f() { std::remove("tmp/program.bas"); }\n',
        )
        errors = inventory.check(self.root)
        self.assertTrue(any(".bas file use" in error for error in errors), errors)

    def test_adapter_marker_rejects_project_tmp(self):
        write(
            self.root,
            "tests/unit/src/test_source_adapter.cpp",
            "// test-inventory: file-adapter\n"
            'void f() { std::ofstream out("tmp/program.bas"); }\n',
        )
        errors = inventory.check(self.root)
        self.assertTrue(any("OS temporary" in error for error in errors), errors)

    def test_adapter_marker_allows_os_temp_directory(self):
        write(
            self.root,
            "tests/unit/src/test_source_adapter.cpp",
            "// test-inventory: file-adapter\n"
            'void f() { std::ofstream out(temp.file("program.bas")); }\n',
        )
        self.assertEqual(inventory.check(self.root), [])

    def test_non_bas_file_io_fails_without_marker(self):
        write(
            self.root,
            "tests/unit/src/test_compile.cpp",
            'void f() { std::ifstream in("fixture.bin"); }\n',
        )
        errors = inventory.check(self.root)
        self.assertTrue(any("filesystem access" in error for error in errors),
                        errors)

    def test_in_memory_source_passes(self):
        write(
            self.root,
            "tests/unit/src/test_compile.cpp",
            'void f() { resolver.add("tmp/program.bas", "10 END"); }\n',
        )
        self.assertEqual(inventory.check(self.root), [])


if __name__ == "__main__":
    unittest.main()
