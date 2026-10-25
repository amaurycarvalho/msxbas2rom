/***
 * @file test_kernel_manifest.cpp
 * @brief Generated kernel module manifest tests
 */

// NOLINTBEGIN

#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "generated_kernel_image.h"
#include "kernel_manifest_generated.h"
#include "kernel_module.h"
#include "kernel_size_map.h"

static KernelModuleManifest generatedManifest() {
  KernelModuleManifest manifest;

  for (unsigned int i = 0; i < KERNEL_MANIFEST_MODULE_COUNT; i++) {
    const GeneratedKernelModuleDefinition& definition =
        kernel_manifest_modules[i];

    KernelModule module;
    module.id = definition.id;
    module.name = definition.name;
    module.owner = definition.owner;
    module.group = definition.group;
    module.classification = definition.classification;
    module.assemblyBase = definition.assembly_base;
    module.fixedAddress = definition.fixed_address;
    module.hasFixedAddress = definition.has_fixed_address != 0;
    module.policy =
        static_cast<KernelPlacementPolicy>(definition.policy);

    REQUIRE(definition.bytes != nullptr);
    module.bytes.assign(definition.bytes,
                        definition.bytes + definition.size);

    for (unsigned int j = 0; j < definition.exports_count; j++) {
      KernelExport entry;
      entry.name = definition.exports[j].name;
      entry.offset = definition.exports[j].offset;
      module.exports.push_back(entry);
    }

    for (unsigned int j = 0; j < definition.imports_count; j++) {
      KernelImport entry;
      entry.target = definition.imports[j].target;
      entry.offset = definition.imports[j].offset;
      entry.kind = static_cast<KernelImportKind>(
          definition.imports[j].kind);
      module.imports.push_back(entry);
    }

    for (unsigned int j = 0; j < definition.relocations_count; j++) {
      KernelRelocation relocation;
      relocation.offset = definition.relocations[j];
      module.relocations.push_back(relocation);
    }

    for (unsigned int j = 0; j < definition.dependencies_count; j++) {
      module.dependencies.push_back(definition.dependencies[j]);
    }

    for (unsigned int j = 0; j < definition.patch_points_count; j++) {
      module.patchPoints.push_back(definition.patch_points[j]);
    }

    manifest.modules.push_back(module);
  }

  for (unsigned int i = 0; i < KERNEL_MANIFEST_FIXED_REGION_COUNT; i++) {
    const GeneratedKernelFixedRegionDefinition& definition =
        kernel_manifest_fixed_regions[i];
    KernelFixedRegion region;
    region.id = definition.id;
    region.start = definition.start;
    region.end = definition.end;
    region.owner = definition.owner;
    manifest.fixedRegions.push_back(region);
  }

  return manifest;
}

TEST_SUITE("Kernel module manifest") {
  TEST_CASE("Generated manifest validates against the schema") {
    KernelModuleManifest manifest = generatedManifest();
    std::string error;

    CHECK(manifest.modules.size() == KERNEL_MANIFEST_MODULE_COUNT);
    CHECK(manifest.fixedRegions.size() ==
          KERNEL_MANIFEST_FIXED_REGION_COUNT);
    CHECK_MESSAGE(manifest.validate(error), error);
  }

  TEST_CASE("Size map covers every module in the kernel page") {
    KernelModuleManifest manifest = generatedManifest();
    unsigned int total = 0;
    unsigned int cursor = 0x4000;

    for (std::size_t i = 0; i < manifest.modules.size(); i++) {
      const KernelModule& module = manifest.modules[i];
      CHECK(module.assemblyBase == cursor);
      CHECK(module.size() > 0);
      cursor += module.size();
      total += module.size();
    }

    CHECK(cursor == 0x8000);
    CHECK(total == 0x4000);
  }

  TEST_CASE("Fixed regions match designated addresses") {
    KernelModuleManifest manifest = generatedManifest();
    const KernelFixedRegion* player = nullptr;
    const KernelFixedRegion* interrupt = nullptr;

    for (std::size_t i = 0; i < manifest.fixedRegions.size(); i++) {
      if (manifest.fixedRegions[i].id == "arkos_player") {
        player = &manifest.fixedRegions[i];
      }
      if (manifest.fixedRegions[i].id == "player_interrupt") {
        interrupt = &manifest.fixedRegions[i];
      }
    }

    REQUIRE(player != nullptr);
    REQUIRE(interrupt != nullptr);
    CHECK(player->start == 0x6481);
    CHECK(player->end == 0x6BD5);
    CHECK(interrupt->start == 0x5659);
    CHECK(interrupt->end == 0x56A6);
  }

  TEST_CASE("Manifest maps dispatch exports to their module") {
    KernelModuleManifest manifest = generatedManifest();
    const KernelModule* trap =
        manifest.findModuleForExport("XBASIC_TRAP_CHECK");

    REQUIRE(trap != nullptr);
    CHECK(trap->id == "90-traps");
    const KernelExport* entry = trap->findExport("XBASIC_TRAP_CHECK");
    REQUIRE(entry != nullptr);
    CHECK(entry->offset < trap->size());
  }

  TEST_CASE("Manifest rejects a module outside the kernel page") {
    KernelModuleManifest manifest = generatedManifest();
    manifest.modules[0].bytes.resize(0x4001);
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("overflows") != std::string::npos);
  }
}

// NOLINTEND
