#!/usr/bin/env python3
"""Unit tests for the regression runner's memory validation helpers."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import runner  # noqa: E402

RAM_SIZE = runner.RAM_SIZE


def make_program() -> runner.Program:
    return runner.Program(
        "TEST", "sample", runner.ROOT / "tests/integration/TEST/sample.bas"
    )


def make_baseline(mask: bytes, values: bytes, ranges=None) -> dict:
    return {
        "ram": {
            "mask": runner.compress_bytes(mask),
            "values": runner.compress_bytes(values),
            "volatile_ranges": ranges or [],
        }
    }


class ProbeTests(unittest.TestCase):
    def test_matching_probe_passes(self):
        ram = bytearray(RAM_SIZE)
        ram[0x48] = 0x07
        ok, checks = runner.compare_probes(
            {"ram_probes": [{"address": "0xC048", "value": "0x07"}]},
            bytes(ram),
        )
        self.assertTrue(ok)
        self.assertTrue(checks[0]["ok"])

    def test_mismatching_probe_fails_with_address_and_values(self):
        ram = bytearray(RAM_SIZE)
        ram[0x48] = 0x07
        ok, checks = runner.compare_probes(
            {"ram_probes": [{"address": "0xC048", "value": "0x08"}]},
            bytes(ram),
        )
        self.assertFalse(ok)
        self.assertFalse(checks[0]["ok"])
        self.assertIn("0xC048", checks[0]["name"])
        self.assertIn("08", checks[0]["detail"])
        self.assertIn("07", checks[0]["detail"])


class SnapshotTests(unittest.TestCase):
    def test_changed_stable_byte_fails(self):
        mask = bytes([0xFF]) * RAM_SIZE
        values = bytes(RAM_SIZE)
        ram = bytearray(values)
        ram[0x100] = 0x42
        ok, reason, _ = runner.compare_ram(
            make_program(),
            {},
            make_baseline(mask, values),
            bytes(ram),
        )
        self.assertFalse(ok)
        self.assertIn("0xC100", reason)

    def test_changed_volatile_byte_is_masked(self):
        mask = bytes([0xFF]) * RAM_SIZE
        values = bytes(RAM_SIZE)
        ram = bytearray(values)
        ram[0x100] = 0x42
        ok, _, _ = runner.compare_ram(
            make_program(),
            {"volatile_ranges": [["0xC100", "0xC100"]]},
            make_baseline(mask, values),
            bytes(ram),
        )
        self.assertTrue(ok)

    def test_baseline_marks_unstable_bytes(self):
        ram_a = bytearray(RAM_SIZE)
        ram_b = bytearray(RAM_SIZE)
        ram_b[0x200] = 0x99
        program = make_program()
        config = {"machine": "M", "volatile_ranges": [], "vram": {}}
        baseline = runner.build_baseline(
            program,
            config,
            [bytes(ram_a), bytes(ram_b)],
            [bytes(16384), bytes(16384)],
            {"trigger": "test"},
            [0, 0],
            2,
        )
        mask = runner.decompress_bytes(baseline["ram"]["mask"])
        values = runner.decompress_bytes(baseline["ram"]["values"])
        self.assertEqual(mask[0x200], 0x00)
        self.assertEqual(mask[0x201], 0xFF)
        self.assertEqual(values[0x201], 0x00)


class VramTests(unittest.TestCase):
    def test_text_extraction(self):
        vram = bytearray(16384)
        name_table = 0x1800
        for index, char in enumerate(b"HELLO"):
            vram[name_table + index] = char
        text = runner.extract_text(
            bytes(vram),
            bytes(RAM_SIZE),
            1,
            {"text": {"name_table": "0x1800", "width": 40, "height": 24}},
        )
        self.assertIn("HELLO", text)

    def test_region_hash_detects_difference(self):
        vram_a = bytearray(16384)
        vram_b = bytearray(16384)
        vram_b[0x10] = 0xFF
        hash_a = runner.sha256_region(bytes(vram_a), 0x0000, 0x100)
        hash_b = runner.sha256_region(bytes(vram_b), 0x0000, 0x100)
        self.assertNotEqual(hash_a, hash_b)

    def test_screen_mode_from_vdp_registers(self):
        self.assertEqual(runner.screen_from_vdp(bytes([0x00, 0xF0]), 0), 0)
        self.assertEqual(runner.screen_from_vdp(bytes([0x00, 0xE0]), 0), 1)
        self.assertEqual(runner.screen_from_vdp(bytes([0x02, 0xE0]), 0), 2)
        self.assertEqual(runner.screen_from_vdp(bytes([0x06, 0x60]), 5), 5)


if __name__ == "__main__":
    unittest.main()
