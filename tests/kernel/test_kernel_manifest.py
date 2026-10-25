#!/usr/bin/env python3
"""Integration tests for the kernel manifest generator."""

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / "scripts"
sys.path.insert(0, str(SCRIPTS))

import generate_kernel_manifest as generator  # noqa: E402

KERNEL_DIR = ROOT / "src" / "infrastructure" / "kernel" / "asm"
SYMBOLS = KERNEL_DIR / "src" / "header.symbols.asm"


class KernelManifestTests(unittest.TestCase):
    def test_current_kernel_manifest_covers_the_page(self):
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)

        self.assertGreater(len(manifest["modules"]), 0)
        total = sum(module["size"] for module in manifest["modules"])
        self.assertEqual(total, 0x4000)
        self.assertEqual(manifest["fixed_regions"][4]["id"], "arkos_player")
        self.assertEqual(manifest["fixed_regions"][4]["start"], 0x6481)

    def test_every_code_label_belongs_to_one_module(self):
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)

        labels = generator.collect_source_labels(KERNEL_DIR / "src" / "header")
        symbols = generator.parse_symbols(SYMBOLS)
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
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)

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
            self.assertEqual(by_owner[owner]["classification"], "fixed")
        self.assertEqual(
            by_owner["31-cmd-tile-sprite"]["classification"], "flex-high"
        )
        self.assertEqual(by_owner["90-traps"]["classification"], "pinned")
        self.assertEqual(by_owner["90-math"]["classification"], "flex-low")
        self.assertEqual(by_owner["90-strings"]["classification"], "flex-low")
        self.assertEqual(
            by_owner["34_file_handling"]["classification"], "flex-low"
        )

    def test_fixed_regions_include_player_regions(self):
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)
        regions = {region["id"]: region for region in manifest["fixed_regions"]}

        self.assertEqual(regions["arkos_player"]["start"], 0x6481)
        self.assertEqual(regions["arkos_player"]["end"], 0x6BD5)
        self.assertEqual(regions["player_interrupt"]["start"], 0x5659)
        self.assertEqual(regions["player_interrupt"]["end"], 0x56A6)

    def test_generated_manifest_exports_dispatch_names(self):
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)

        names = {
            entry["name"]
            for module in manifest["modules"]
            for entry in module["exports"]
        }
        self.assertIn("XBASIC_TRAP_CHECK", names)
        self.assertIn("MR_CALL", names)

    def test_manifest_records_address_of_imports(self):
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)
        modules = {module["owner"]: module for module in manifest["modules"]}

        imports = modules["30_basic_helpers"]["imports"]
        string = [entry for entry in imports if entry["target"] == "C7EF2"]
        self.assertEqual(len(string), 1)
        self.assertEqual(string[0]["kind"], "jump")
        self.assertGreater(string[0]["offset"], 0)
        usr = [entry for entry in imports if entry["target"] == "usr0"]
        self.assertEqual(len(usr), 0)

    def test_manifest_imports_only_movable_modules(self):
        manifest = generator.build_manifest(KERNEL_DIR, SYMBOLS)
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


if __name__ == "__main__":
    unittest.main()
