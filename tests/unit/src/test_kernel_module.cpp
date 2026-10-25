/***
 * @file test_kernel_module.cpp
 * @brief Kernel module manifest, relocation, and size map tests
 */

// NOLINTBEGIN

#include <map>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "kernel_module.h"
#include "kernel_size_map.h"

static KernelModule makeModule(const std::string& id,
                               unsigned int address) {
  KernelModule module;
  module.id = id;
  module.name = id;
  module.owner = id;
  module.bytes.assign(4, 0x00);
  module.assemblyBase = address;
  module.fixedAddress = address;
  module.hasFixedAddress = true;
  module.policy = KernelPlacementPolicy::Fixed;

  KernelExport entry;
  entry.name = id + "_entry";
  entry.offset = 0;
  module.exports.push_back(entry);
  return module;
}

static std::vector<unsigned char> assemblyAt(unsigned int base) {
  std::vector<unsigned char> bytes;
  bytes.push_back(0xCD);  // call
  unsigned int internal = base + 0x0010;
  bytes.push_back(internal & 0xFF);
  bytes.push_back((internal >> 8) & 0xFF);
  bytes.push_back(0x21);  // ld hl, BIOS
  bytes.push_back(0x00);
  bytes.push_back(0x00);
  bytes.push_back(0xC9);  // ret
  bytes.resize(0x20, 0x00);
  bytes[0x10] = 0xC9;
  return bytes;
}

TEST_SUITE("Kernel module manifest") {
  TEST_CASE("Valid manifest passes") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("core", 0x4000));
    std::string error;

    CHECK(manifest.validate(error) == true);
    CHECK(error.empty() == true);
  }

  TEST_CASE("Duplicate module ids are rejected") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("core", 0x4000));
    manifest.modules.push_back(makeModule("core", 0x4010));
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("duplicate module id") != std::string::npos);
  }

  TEST_CASE("Missing dependency is rejected") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("core", 0x4000));
    manifest.modules[0].dependencies.push_back("missing");
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("missing module") != std::string::npos);
  }

  TEST_CASE("Dependency cycles are rejected") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("a", 0x4000));
    manifest.modules.push_back(makeModule("b", 0x4010));
    manifest.modules[0].dependencies.push_back("b");
    manifest.modules[1].dependencies.push_back("a");
    std::string error;

    CHECK(manifest.validateAcyclic(error) == false);
    CHECK(error.find("cycle") != std::string::npos);
  }

  TEST_CASE("Fixed module outside the kernel page is rejected") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("core", 0x8000));
    manifest.modules[0].fixedAddress = 0x8000;
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("outside the kernel page") != std::string::npos);
  }

  TEST_CASE("Relocation and patch offsets are bounds checked") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("core", 0x4000));
    KernelRelocation relocation;
    relocation.offset = 3;
    manifest.modules[0].relocations.push_back(relocation);
    manifest.modules[0].patchPoints.push_back(4);
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("relocation offset") != std::string::npos);
  }

  TEST_CASE("Fixed regions cannot overlap") {
    KernelModuleManifest manifest;
    manifest.modules.push_back(makeModule("core", 0x4000));
    KernelFixedRegion first = {"one", 0x4000, 0x4100, "core"};
    KernelFixedRegion second = {"two", 0x40FF, 0x4200, "core"};
    manifest.fixedRegions.push_back(first);
    manifest.fixedRegions.push_back(second);
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("overlap") != std::string::npos);
  }
}

TEST_SUITE("Kernel module relocation") {
  TEST_CASE("Dual-base diff records only relocatable words") {
    std::vector<unsigned char> at4000 = assemblyAt(0x4000);
    std::vector<unsigned char> at4100 = assemblyAt(0x4100);
    KernelRelocationDiff diff;
    std::string error;

    REQUIRE(diffRelocations(at4000, at4100, 0x0100, diff, error) == true);
    REQUIRE(diff.offsets.size() == 1);
    CHECK(diff.offsets[0] == 1);
    INFO(error);
  }

  TEST_CASE("Relocated module matches assembly at the final base") {
    std::vector<unsigned char> at4000 = assemblyAt(0x4000);
    std::vector<unsigned char> at4100 = assemblyAt(0x4100);
    KernelRelocationDiff diff;
    std::string error;
    REQUIRE(diffRelocations(at4000, at4100, 0x0100, diff, error) == true);

    std::vector<KernelRelocation> relocations;
    for (std::size_t i = 0; i < diff.offsets.size(); i++) {
      KernelRelocation relocation;
      relocation.offset = diff.offsets[i];
      relocations.push_back(relocation);
    }

    REQUIRE(applyRelocations(at4000, 0x4000, 0x4200, relocations, error) ==
            true);
    CHECK(at4000 == assemblyAt(0x4200));
  }

  TEST_CASE("Kernel, BIOS, and RAM references are excluded from relocation") {
    std::vector<unsigned char> at4000(16, 0x00);
    std::vector<unsigned char> at4100(16, 0x00);

    at4000[1] = 0x10;
    at4000[2] = 0x40;
    at4100[1] = 0x10;
    at4100[2] = 0x41;

    at4000[4] = 0x00;
    at4000[5] = 0x40;
    at4100[4] = 0x00;
    at4100[5] = 0x40;

    at4000[7] = 0xA2;
    at4000[8] = 0x00;
    at4100[7] = 0xA2;
    at4100[8] = 0x00;

    at4000[10] = 0x00;
    at4000[11] = 0xF0;
    at4100[10] = 0x00;
    at4100[11] = 0xF0;

    KernelRelocationDiff diff;
    std::string error;
    REQUIRE(diffRelocations(at4000, at4100, 0x0100, diff, error) == true);
    REQUIRE(diff.offsets.size() == 1);
    CHECK(diff.offsets[0] == 1);

    std::vector<KernelRelocation> relocations;
    KernelRelocation relocation;
    relocation.offset = diff.offsets[0];
    relocations.push_back(relocation);
    REQUIRE(applyRelocations(at4000, 0x4000, 0x4200, relocations, error) ==
            true);

    CHECK(at4000[1] == 0x10);
    CHECK(at4000[2] == 0x42);
    CHECK(at4000[4] == 0x00);
    CHECK(at4000[5] == 0x40);
    CHECK(at4000[7] == 0xA2);
    CHECK(at4000[8] == 0x00);
    CHECK(at4000[10] == 0x00);
    CHECK(at4000[11] == 0xF0);
  }

  TEST_CASE("Fixed references are not relocated") {
    std::vector<unsigned char> bytes = assemblyAt(0x4000);
    std::vector<KernelRelocation> relocations;
    KernelRelocation relocation;
    relocation.offset = 1;
    relocations.push_back(relocation);
    std::string error;

    REQUIRE(applyRelocations(bytes, 0x4000, 0x4100, relocations, error) ==
            true);
    CHECK(bytes[4] == 0x00);
    CHECK(bytes[5] == 0x00);
    CHECK(bytes[1] == 0x10);
    CHECK(bytes[2] == 0x41);
  }

  TEST_CASE("Relocation bounds are validated") {
    std::vector<unsigned char> bytes(2, 0x00);
    std::vector<KernelRelocation> relocations;
    KernelRelocation relocation;
    relocation.offset = 1;
    relocations.push_back(relocation);
    std::string error;

    CHECK(applyRelocations(bytes, 0x4000, 0x4100, relocations, error) ==
          false);
    CHECK(error.find("out of range") != std::string::npos);
  }
}

TEST_SUITE("Kernel module imports") {
  TEST_CASE("Valid cross-module import passes validation") {
    KernelModuleManifest manifest;
    KernelModule a = makeModule("a", 0x4000);
    KernelModule b = makeModule("b", 0x4010);
    KernelImport imported;
    imported.target = "b_entry";
    imported.offset = 1;
    imported.kind = KernelImportKind::Call;
    a.imports.push_back(imported);
    manifest.modules.push_back(a);
    manifest.modules.push_back(b);
    std::string error;

    CHECK(manifest.validate(error) == true);
  }

  TEST_CASE("Import of a missing export is rejected") {
    KernelModuleManifest manifest;
    KernelModule a = makeModule("a", 0x4000);
    KernelImport imported;
    imported.target = "missing_entry";
    imported.offset = 1;
    imported.kind = KernelImportKind::Address;
    a.imports.push_back(imported);
    manifest.modules.push_back(a);
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("imports missing export") != std::string::npos);
  }

  TEST_CASE("Import offsets are bounds checked") {
    KernelModuleManifest manifest;
    KernelModule a = makeModule("a", 0x4000);
    KernelModule b = makeModule("b", 0x4010);
    KernelImport imported;
    imported.target = "b_entry";
    imported.offset = 3;
    imported.kind = KernelImportKind::Jump;
    a.imports.push_back(imported);
    manifest.modules.push_back(a);
    manifest.modules.push_back(b);
    std::string error;

    CHECK(manifest.validate(error) == false);
    CHECK(error.find("import offset") != std::string::npos);
  }

  TEST_CASE("Import fixup writes the resolved stub address") {
    std::vector<unsigned char> bytes(4, 0x00);
    KernelImport imported;
    imported.target = "b_entry";
    imported.offset = 1;
    imported.kind = KernelImportKind::Call;
    std::vector<KernelImport> imports;
    imports.push_back(imported);
    std::map<std::string, unsigned int> stubs;
    stubs["b_entry"] = 0x4A86;
    std::string error;

    REQUIRE(applyModuleImports(bytes, imports, stubs, error) == true);
    CHECK(bytes[1] == 0x86);
    CHECK(bytes[2] == 0x4A);
  }

  TEST_CASE("Missing import stub is rejected") {
    std::vector<unsigned char> bytes(4, 0x00);
    KernelImport imported;
    imported.target = "b_entry";
    imported.offset = 1;
    imported.kind = KernelImportKind::Call;
    std::vector<KernelImport> imports;
    imports.push_back(imported);
    std::map<std::string, unsigned int> stubs;
    std::string error;

    CHECK(applyModuleImports(bytes, imports, stubs, error) == false);
    CHECK(error.find("no stub address") != std::string::npos);
  }
}

TEST_SUITE("Kernel size map") {
  TEST_CASE("Module extents partition the kernel page") {
    std::vector<KernelSymbol> symbols;
    symbols.push_back({"a1", 0x4000, "a"});
    symbols.push_back({"a2", 0x4010, "a"});
    symbols.push_back({"b1", 0x4020, "b"});
    symbols.push_back({"b2", 0x4030, "b"});

    std::vector<KernelModuleExtent> extents =
        KernelSizeMap::moduleExtents(symbols, 0x4000, 0x4040);

    REQUIRE(extents.size() == 2);
    CHECK(extents[0].owner == "a");
    CHECK(extents[0].start == 0x4000);
    CHECK(extents[0].end == 0x4020);
    CHECK(extents[0].size() == 0x20);
    CHECK(extents[1].owner == "b");
    CHECK(extents[1].start == 0x4020);
    CHECK(extents[1].end == 0x4040);
  }

  TEST_CASE("Included modules split the outer module extent") {
    std::vector<KernelSymbol> symbols;
    symbols.push_back({"outer.first", 0x4000, "outer"});
    symbols.push_back({"inner.first", 0x4010, "inner"});
    symbols.push_back({"outer.last", 0x4020, "outer"});

    std::vector<KernelModuleExtent> extents =
        KernelSizeMap::moduleExtents(symbols, 0x4000, 0x4040);

    REQUIRE(extents.size() == 3);
    CHECK(extents[0].owner == "outer");
    CHECK(extents[0].start == 0x4000);
    CHECK(extents[0].end == 0x4010);
    CHECK(extents[1].owner == "inner");
    CHECK(extents[1].start == 0x4010);
    CHECK(extents[1].end == 0x4020);
    CHECK(extents[2].owner == "outer");
    CHECK(extents[2].start == 0x4020);
    CHECK(extents[2].end == 0x4040);
  }

  TEST_CASE("Routine extents stop at the next routine or module end") {
    std::vector<KernelSymbol> symbols;
    symbols.push_back({"first", 0x4000, "a"});
    symbols.push_back({"first.local", 0x4004, "a"});
    symbols.push_back({"second", 0x4008, "a"});

    std::vector<KernelModuleExtent> modules =
        KernelSizeMap::moduleExtents(symbols, 0x4000, 0x4010);
    std::vector<KernelRoutineExtent> routines =
        KernelSizeMap::routineExtents(symbols, modules);

    REQUIRE(routines.size() == 3);
    CHECK(routines[0].name == "first");
    CHECK(routines[0].size() == 4);
    CHECK(routines[1].name == "first.local");
    CHECK(routines[1].size() == 4);
    CHECK(routines[2].name == "second");
    CHECK(routines[2].size() == 8);
  }

  TEST_CASE("Symbols outside the kernel page are ignored") {
    std::vector<KernelSymbol> symbols;
    symbols.push_back({"virtual", 0x0000, "runtime"});
    symbols.push_back({"resident", 0x5000, "runtime"});
    symbols.push_back({"ram", 0xF000, "runtime"});

    std::vector<KernelModuleExtent> extents =
        KernelSizeMap::moduleExtents(symbols, 0x4000, 0x8000);

    REQUIRE(extents.size() == 1);
    CHECK(extents[0].start == 0x5000);
    CHECK(extents[0].end == 0x8000);
  }
}

// NOLINTEND
