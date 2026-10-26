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


class ParsingTests(unittest.TestCase):
    def test_screen_with_space(self):
        self.assertEqual(runner.screen_modes("10 SCREEN 4"), (4, 4))

    def test_screen_without_space(self):
        self.assertEqual(runner.screen_modes("60 SCREEN8:CLS"), (8, 8))

    def test_screen_12(self):
        self.assertEqual(runner.screen_modes("10 SCREEN 12"), (12, 12))

    def test_non_literal_screen_is_ignored(self):
        self.assertEqual(runner.screen_modes("10 SCREEN (M)"), (0, 0))


class MachineSelectionTests(unittest.TestCase):
    def select(self, text: str, **config):
        config = dict(config)
        program = make_program()
        return (
            runner.machine_class_for(program, config, text),
            runner.machine_for(program, config, text),
        )

    def test_msx1_program_uses_msx1_default(self):
        self.assertEqual(
            self.select("10 SCREEN 1"),
            ("msx1", "Sharp_HB-8000_1.2"),
        )

    def test_msx2_requirement_uses_msx2_default(self):
        self.assertEqual(
            self.select("10 SCREEN 1", requires="msx2"),
            ("msx2", "Panasonic_FS-A1WX"),
        )

    def test_screen_4_uses_msx2plus_default(self):
        self.assertEqual(
            self.select("10 SCREEN 4"),
            ("msx2plus", "Panasonic_FS-A1WX"),
        )

    def test_screen_without_space_uses_msx2plus_default(self):
        self.assertEqual(
            self.select("10 SCREEN8"),
            ("msx2plus", "Panasonic_FS-A1WX"),
        )

    def test_turbo_command_without_requirement_uses_msx2plus(self):
        self.assertEqual(
            self.select("10 CMD TURBO 1"),
            ("msx2plus", "Panasonic_FS-A1WX"),
        )

    def test_turbo_r_requirement_uses_turbo_r_default(self):
        self.assertEqual(
            self.select("10 SCREEN 1", requires="turboR"),
            ("turboR", "Panasonic_FS-A1GT"),
        )

    def test_explicit_machine_override_wins(self):
        self.assertEqual(
            self.select("10 SCREEN 1", requires="turboR", machine="Custom_MSX"),
            ("turboR", "Custom_MSX"),
        )


class ManifestMachineAssignmentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = runner.load_manifest()
        cls.programs = {
            (program.scenario, program.rel): program
            for program in runner.discover_programs()
        }

    def selection(self, label: str):
        scenario, rel = label.split("/", 1)
        program = self.programs[(scenario, rel)]
        config = runner.program_config(self.manifest, program)
        text = program.source.read_text(encoding="utf-8", errors="replace")
        return (
            runner.machine_class_for(program, config, text),
            runner.machine_for(program, config, text),
        )

    def test_turbo_r_feature_programs_select_turbo_r(self):
        for label in ("MISC/test1", "MISC/test64", "MISC/test65"):
            with self.subTest(program=label):
                self.assertEqual(
                    self.selection(label),
                    ("turboR", "Panasonic_FS-A1GT"),
                )

    def test_performance_only_turbo_programs_stay_on_msx2plus(self):
        for label in (
            "GRAPH/tree",
            "GRAPH/mexhat",
            "GRAPH/test78",
            "IO/test48",
            "MATH/cos",
        ):
            with self.subTest(program=label):
                self.assertEqual(
                    self.selection(label),
                    ("msx2plus", "Panasonic_FS-A1WX"),
                )


class BaselineMachineTests(unittest.TestCase):
    def test_matching_machine_passes(self):
        ok, reason = runner.check_baseline_machine(
            {"machine": "Panasonic_FS-A1WX"},
            {"machine": "Panasonic_FS-A1WX"},
        )
        self.assertTrue(ok)
        self.assertEqual(reason, "")

    def test_mismatching_machine_fails_with_regenerate_instruction(self):
        ok, reason = runner.check_baseline_machine(
            {"machine": "Panasonic_FS-A1WX"},
            {"machine": "Sharp_HB-8000_1.2"},
        )
        self.assertFalse(ok)
        self.assertIn("Sharp_HB-8000_1.2", reason)
        self.assertIn("Panasonic_FS-A1WX", reason)
        self.assertIn("regenerate", reason)

    def test_missing_baseline_machine_is_allowed(self):
        ok, _ = runner.check_baseline_machine(
            {"machine": "Panasonic_FS-A1WX"}, {}
        )
        self.assertTrue(ok)


class BreakpointTests(unittest.TestCase):
    def test_end_stmt_prefers_pre_cleanup_call(self):
        self.assertEqual(
            runner.breakpoints_from_symbols({"END_STMT": 0x28063}),
            [("END_STMT+2", 0x8065)],
        )

    def test_end_pgm_is_used_when_end_stmt_is_absent(self):
        self.assertEqual(
            runner.breakpoints_from_symbols({"END_PGM": 0x2828B}),
            [("END_PGM", 0x828B)],
        )

    def test_end_pgm_is_not_added_when_end_stmt_exists(self):
        self.assertEqual(
            runner.breakpoints_from_symbols(
                {"END_STMT": 0x28063, "END_PGM": 0x2828B}
            ),
            [("END_STMT+2", 0x8065)],
        )


class ScreenModeConsistencyTests(unittest.TestCase):
    def test_matching_modes_have_no_conflict(self):
        self.assertEqual(runner.screen_mode_consistency(5, 5), (5, None))

    def test_graphical_source_conflicting_with_text_vdp_keeps_source_layout(self):
        layout, message = runner.screen_mode_consistency(5, 0)
        self.assertEqual(layout, 5)
        self.assertIn("SCREEN 5", message)
        self.assertIn("mode 0", message)

    def test_graphical_vdp_mode_wins_over_different_source_mode(self):
        layout, message = runner.screen_mode_consistency(5, 4)
        self.assertEqual(layout, 4)
        self.assertIn("SCREEN 5", message)
        self.assertIn("mode 4", message)

    def test_missing_source_mode_has_no_conflict(self):
        self.assertEqual(runner.screen_mode_consistency(None, 0), (0, None))


class FallbackTimingTests(unittest.TestCase):
    def test_short_program_fallback_uses_machine_boot_minimum(self):
        self.assertEqual(
            runner.effective_fallback_seconds(
                {"fallback_seconds": 8}, "msx2plus"
            ),
            15,
        )

    def test_long_program_fallback_wins(self):
        self.assertEqual(
            runner.effective_fallback_seconds(
                {"fallback_seconds": 20}, "msx2plus"
            ),
            20,
        )

    def test_msx1_boot_minimum_is_ten_seconds(self):
        self.assertEqual(
            runner.effective_fallback_seconds(
                {"fallback_seconds": 8}, "msx1"
            ),
            10,
        )

    def test_manifest_can_override_machine_boot_minimum(self):
        self.assertEqual(
            runner.effective_fallback_seconds(
                {
                    "fallback_seconds": 8,
                    "machine_boot_seconds": {"msx2plus": 12},
                },
                "msx2plus",
            ),
            12,
        )


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


class VramProbeTests(unittest.TestCase):
    def test_matching_probe_passes(self):
        vram = bytearray(0x4000)
        vram[0x1C10] = 0x6C
        ok, checks = runner.compare_vram_probes(
            {"vram": {"probes": [{"address": "0x1C10", "value": "0x6C"}]}},
            bytes(vram),
        )
        self.assertTrue(ok)
        self.assertTrue(checks[0]["ok"])
        self.assertIn("0x1C10", checks[0]["name"])

    def test_mismatching_probe_fails_with_offset_and_bytes(self):
        vram = bytearray(0x4000)
        vram[0x1C10] = 0x0C
        ok, checks = runner.compare_vram_probes(
            {"vram": {"probes": [{"address": "0x1C10", "value": "0x6C"}]}},
            bytes(vram),
        )
        self.assertFalse(ok)
        self.assertFalse(checks[0]["ok"])
        self.assertIn("0x1C10", checks[0]["name"])
        self.assertIn("6C", checks[0]["detail"])
        self.assertIn("0C", checks[0]["detail"])

    def test_multi_byte_probe(self):
        vram = bytearray(0x4000)
        vram[0x1C10:0x1C13] = bytes([0x6C, 0x74, 0x74])
        ok, checks = runner.compare_vram_probes(
            {"vram": {"probes": [{"address": "0x1C10", "bytes": "6C 74 74"}]}},
            bytes(vram),
        )
        self.assertTrue(ok)
        self.assertTrue(checks[0]["ok"])

    def test_probe_outside_captured_vram_is_rejected(self):
        vram = bytes(0x4000)
        with self.assertRaises(SystemExit):
            runner.compare_vram_probes(
                {"vram": {"probes": [{"address": "0x4000", "value": "0x00"}]}},
                vram,
            )

    def test_probe_range_exceeding_vram_is_rejected(self):
        vram = bytes(0x4000)
        with self.assertRaises(SystemExit):
            runner.compare_vram_probes(
                {"vram": {"probes": [{"address": "0x3FFF", "bytes": "00 00"}]}},
                vram,
            )

    def test_volatile_program_still_evaluates_probes(self):
        vram = bytearray(0x4000)
        vram[0x1C10] = 0x6C
        config = {"vram": {"probes": [{"address": "0x1C10", "value": "0x6C"}]}}
        ok, checks = runner.compare_vram(
            make_program(), config, {}, bytes(vram), bytes(RAM_SIZE), 2, True
        )
        self.assertTrue(ok)
        self.assertTrue(any(check["name"] == "vram_probe_0x1C10" for check in checks))

    def test_volatile_program_probe_mismatch_fails(self):
        vram = bytearray(0x4000)
        vram[0x1C10] = 0x0C
        config = {"vram": {"probes": [{"address": "0x1C10", "value": "0x6C"}]}}
        ok, checks = runner.compare_vram(
            make_program(), config, {}, bytes(vram), bytes(RAM_SIZE), 2, True
        )
        self.assertFalse(ok)
        self.assertFalse(checks[0]["ok"])


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


class VdpModeTests(unittest.TestCase):
    OBSERVED_PAIRS = (
        ((0x00, 0x70), 0),
        ((0x00, 0x60), 1),
        ((0x02, 0x60), 2),
        ((0x00, 0x68), 3),
        ((0x04, 0x60), 4),
        ((0x06, 0x60), 5),
        ((0x08, 0x60), 6),
        ((0x0A, 0x60), 7),
        ((0x0E, 0x60), 8),
    )

    def test_observed_register_pairs(self):
        for (r0, r1), expected in self.OBSERVED_PAIRS:
            with self.subTest(r0=r0, r1=r1):
                self.assertEqual(
                    runner.screen_from_vdp(bytes([r0, r1]), 0),
                    expected,
                )

    def test_screen_9_to_12_uses_source_fallback(self):
        for screen in (9, 10, 11, 12):
            with self.subTest(screen=screen):
                self.assertEqual(
                    runner.screen_from_vdp(bytes([0x0E, 0x60]), screen),
                    screen,
                )

    def test_screen_8_when_fallback_is_not_9_to_12(self):
        self.assertEqual(runner.screen_from_vdp(bytes([0x0E, 0x60]), 0), 8)
        self.assertEqual(runner.screen_from_vdp(bytes([0x0E, 0x60]), 5), 8)

    def test_screen_5_capture_reports_5_not_2(self):
        self.assertEqual(runner.screen_from_vdp(bytes([0x06, 0x60]), 2), 5)

    def test_masked_bits_are_ignored(self):
        self.assertEqual(runner.screen_from_vdp(bytes([0x06, 0xE0]), 0), 5)
        self.assertEqual(runner.screen_from_vdp(bytes([0x00, 0xF0]), 0), 0)


if __name__ == "__main__":
    unittest.main()
