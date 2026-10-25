#!/usr/bin/env python3
"""Tests for the kernel module manifest generator."""

import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(SCRIPTS))

import generate_kernel_manifest as generator  # noqa: E402

ROOT = SCRIPTS.parent


class GeneratorTests(unittest.TestCase):
    def test_source_labels_skip_equates(self):
        with tempfile.TemporaryDirectory() as directory:
            header = Path(directory)
            (header / "10_module.asm").write_text(
                "VALUE: equ 0x1234\n"
                "start:\n"
                "  nop\n"
                "inner.loop:\n"
                "  ret\n",
                encoding="utf-8",
            )

            labels = generator.collect_source_labels(header)

        self.assertNotIn("VALUE", labels)
        self.assertEqual(labels["start"], "10_module")
        self.assertEqual(labels["inner.loop"], "10_module")

    def test_module_runs_partition_the_kernel_page(self):
        symbols = [
            {"name": "a1", "address": 0x4000, "owner": "a"},
            {"name": "a2", "address": 0x4010, "owner": "a"},
            {"name": "b1", "address": 0x4020, "owner": "b"},
        ]

        runs = generator.build_module_runs(symbols)

        self.assertEqual(len(runs), 2)
        self.assertEqual(runs[0]["owner"], "a")
        self.assertEqual(runs[0]["start"], 0x4000)
        self.assertEqual(runs[0]["end"], 0x4020)
        self.assertEqual(runs[1]["owner"], "b")
        self.assertEqual(runs[1]["start"], 0x4020)
        self.assertEqual(runs[1]["end"], generator.KERNEL_END)

    def test_included_module_splits_the_outer_run(self):
        symbols = [
            {"name": "outer.first", "address": 0x4000, "owner": "outer"},
            {"name": "inner.first", "address": 0x4010, "owner": "inner"},
            {"name": "outer.last", "address": 0x4020, "owner": "outer"},
        ]

        runs = generator.build_module_runs(symbols)

        self.assertEqual(len(runs), 3)
        self.assertEqual(
            [(run["owner"], run["start"], run["end"]) for run in runs],
            [
                ("outer", 0x4000, 0x4010),
                ("inner", 0x4010, 0x4020),
                ("outer", 0x4020, generator.KERNEL_END),
            ],
        )

    def test_isolation_rejects_direct_cross_module_reference(self):
        sources = {
            "a": "start:\n  call b_entry\n  ret\n",
            "b": "b_entry:\n  ret\n",
        }
        owners = {"start": "a", "b_entry": "b"}

        with self.assertRaises(ValueError) as context:
            generator.validate_module_isolation(sources, owners)
        self.assertIn("a references b_entry", str(context.exception))

    def test_isolation_accepts_intra_module_reference(self):
        sources = {
            "a": "start:\n  call a_helper\n  ret\na_helper:\n  ret\n",
        }
        owners = {"start": "a", "a_helper": "a"}

        self.assertTrue(generator.validate_module_isolation(sources, owners))

    def test_pinned_player_symbols_are_checked(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols_path = kernel_dir / "src" / "header.symbols.asm"
        symbols = generator.parse_symbols(symbols_path)
        symbols["PLY_AKM_Play"] += 1

        with self.assertRaises(ValueError):
            generator.build_fixed_regions(symbols, kernel_dir)

    def test_source_references_resolve_equ_aliases(self):
        labels = {"start": "a", "other_entry": "b"}
        aliases = {"ALIAS_ENTRY": "other_entry"}
        references = generator.source_module_references(
            "a",
            "start:\n  call ALIAS_ENTRY\n",
            labels,
            aliases,
            {"b": "flex-low"},
            movable_only=True,
            include_addresses=False,
        )

        self.assertEqual(
            references, [("other_entry", "b", "call")]
        )

    def test_cross_module_jr_is_rejected(self):
        labels = {"start": "a", "other_entry": "b"}
        with self.assertRaises(ValueError):
            generator.source_module_references(
                "a",
                "start:\n  jr other_entry\n",
                labels,
                {},
                {"b": "flex-low"},
                movable_only=True,
                include_addresses=False,
            )

    def test_validate_generated_imports_rejects_fixed_target(self):
        modules = [
            {
                "id": "a",
                "owner": "a",
                "exports": [{"name": "a_entry"}],
                "imports": [{"target": "b_entry", "offset": 0,
                             "kind": "call"}],
            },
            {
                "id": "b",
                "owner": "b",
                "exports": [{"name": "b_entry"}],
                "imports": [],
            },
        ]
        with self.assertRaises(ValueError):
            generator.validate_generated_imports(
                modules, {"a": "flex-high", "b": "fixed"}
            )


if __name__ == "__main__":
    unittest.main()
