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

    def test_current_kernel_manifest_covers_the_page(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols)

        self.assertGreater(len(manifest["modules"]), 0)
        total = sum(module["size"] for module in manifest["modules"])
        self.assertEqual(total, 0x4000)
        self.assertEqual(manifest["fixed_regions"][4]["id"], "arkos_player")
        self.assertEqual(manifest["fixed_regions"][4]["start"], 0x6481)

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

    def test_every_code_label_belongs_to_one_module(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols_path = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols_path)

        labels = generator.collect_source_labels(
            kernel_dir / "src" / "header"
        )
        symbols = generator.parse_symbols(symbols_path)
        code_labels = {
            name
            for name in labels
            if name in symbols
            and generator.KERNEL_START <= symbols[name] < generator.KERNEL_END
        }

        for label in code_labels:
            owners = [
                module["owner"]
                for module in manifest["modules"]
                if any(
                    entry["name"] == label for entry in module["exports"]
                )
            ]
            self.assertEqual(len(owners), 1, label)

    def test_group_classification_is_complete_and_prioritized(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols_path = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols_path)

        by_owner = {module["owner"]: module for module in manifest["modules"]}
        for module in manifest["modules"]:
            self.assertIn("group", module)
            self.assertIn("classification", module)

        for owner in (
            "31-cmd-game-graphics",
            "90-graphics",
            "31-cmd-cold-graphics",
            "31-cmd-mtf",
            "32_usr",
            "70_pletter",
        ):
            self.assertEqual(
                by_owner[owner]["classification"], "fixed"
            )
        self.assertEqual(
            by_owner["31-cmd-tile-sprite"]["classification"], "flex-high"
        )
        self.assertEqual(by_owner["90-traps"]["classification"], "pinned")
        self.assertEqual(
            by_owner["90-math"]["classification"], "flex-low"
        )
        self.assertEqual(
            by_owner["90-strings"]["classification"], "flex-low"
        )
        self.assertEqual(
            by_owner["34_file_handling"]["classification"], "flex-low"
        )

    def test_fixed_regions_include_player_regions(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols_path = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols_path)
        regions = {region["id"]: region for region in manifest["fixed_regions"]}

        self.assertEqual(regions["arkos_player"]["start"], 0x6481)
        self.assertEqual(regions["arkos_player"]["end"], 0x6BD5)
        self.assertEqual(regions["player_interrupt"]["start"], 0x5659)
        self.assertEqual(regions["player_interrupt"]["end"], 0x56A6)

    def test_generated_manifest_exports_dispatch_names(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols)

        names = {
            entry["name"]
            for module in manifest["modules"]
            for entry in module["exports"]
        }
        self.assertIn("XBASIC_TRAP_CHECK", names)
        self.assertIn("MR_CALL", names)


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

    def test_manifest_records_address_of_imports(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols)
        modules = {module["owner"]: module for module in manifest["modules"]}

        imports = modules["30_basic_helpers"]["imports"]
        string = [entry for entry in imports if entry["target"] == "C7EF2"]
        self.assertEqual(len(string), 1)
        self.assertEqual(string[0]["kind"], "jump")
        self.assertGreater(string[0]["offset"], 0)
        usr = [entry for entry in imports if entry["target"] == "usr0"]
        self.assertEqual(len(usr), 0)

    def test_manifest_imports_only_movable_modules(self):
        kernel_dir = ROOT / "src" / "infrastructure" / "kernel" / "asm"
        symbols = kernel_dir / "src" / "header.symbols.asm"
        manifest = generator.build_manifest(kernel_dir, symbols)
        owner_by_export = {}
        classification = {}
        for module in manifest["modules"]:
            classification[module["owner"]] = module["classification"]
            for entry in module["exports"]:
                owner_by_export[entry["name"]] = module["owner"]
        for module in manifest["modules"]:
            for entry in module["imports"]:
                target_owner = owner_by_export[entry["target"]]
                self.assertNotIn(
                    classification[target_owner], ("fixed", "pinned")
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
