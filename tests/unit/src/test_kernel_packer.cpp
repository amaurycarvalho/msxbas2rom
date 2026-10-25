/***
 * @file test_kernel_packer.cpp
 * @brief Kernel module packing, closure, and budget tests
 */

// NOLINTBEGIN

#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "kernel_module_library.h"
#include "kernel_packer.h"

static KernelModule makeModule(const std::string& id, unsigned int size,
                               KernelPlacementPolicy policy,
                               unsigned int fixedAddress = 0) {
  KernelModule module;
  module.id = id;
  module.name = id;
  module.owner = id;
  module.bytes.assign(size, 0);
  module.assemblyBase = fixedAddress;
  module.fixedAddress = fixedAddress;
  module.hasFixedAddress = fixedAddress != 0;
  module.policy = policy;

  KernelExport entry;
  entry.name = id + "_entry";
  entry.offset = 0;
  module.exports.push_back(entry);
  return module;
}

static KernelPackingRequest baseRequest(
    const InMemoryKernelModuleLibrary& library) {
  KernelPackingRequest request;
  request.library = &library;
  request.kernelStart = 0x4000;
  request.kernelEnd = 0x4100;
  request.dispatcherAddress = 0x4000;
  request.tableBaseAddress = 0x40E0;
  request.romType = KernelRomType::Plain;
  request.overflowStart = 0x8000;
  request.overflowCapacity = 0x4000;
  return request;
}

TEST_SUITE("Kernel packer") {
  TEST_CASE("In-memory library supplies deterministic modules") {
    InMemoryKernelModuleLibrary library;
    library.addModule(makeModule("core", 4, KernelPlacementPolicy::Fixed,
                                 0x4000));
    library.addDispatch(0, "core_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x4010;
    request.kernelEnd = 0x4040;

    KernelPackingResult result = KernelPacker::pack(request);
    INFO(result.error);
    REQUIRE(result.ok == true);
    REQUIRE(result.dispatches.size() == 1);
    CHECK(result.dispatches[0].moduleId == "core");
    CHECK(result.dispatches[0].paged == false);
  }

  TEST_CASE("Only used dispatch entries receive stubs") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40F0, "test"});
    library.addModule(makeModule("used", 4, KernelPlacementPolicy::FlexHigh));
    library.addModule(
        makeModule("unused", 4, KernelPlacementPolicy::FlexHigh));
    library.addDispatch(0, "used_entry");
    library.addDispatch(1, "unused_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x40F0;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);
    REQUIRE(result.dispatches.size() == 1);
    CHECK(result.dispatches[0].exportName == "used_entry");
    CHECK(result.tableSize == 3);
    for (std::size_t i = 0; i < result.placements.size(); i++) {
      CHECK(result.placements[i].moduleId != "unused");
    }
  }

  TEST_CASE("Dependency closure includes transitive dependencies") {
    InMemoryKernelModuleLibrary library;
    KernelModule a = makeModule("a", 4, KernelPlacementPolicy::FlexHigh);
    KernelModule b = makeModule("b", 4, KernelPlacementPolicy::FlexHigh);
    KernelModule c = makeModule("c", 4, KernelPlacementPolicy::FlexHigh);
    a.dependencies.push_back("b");
    b.dependencies.push_back("c");
    library.addModule(a);
    library.addModule(b);
    library.addModule(c);
    library.addDispatch(0, "a_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x4000;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);
    CHECK(result.placements.size() == 3);
  }

  TEST_CASE("Missing dependency fails the build") {
    InMemoryKernelModuleLibrary library;
    KernelModule a = makeModule("a", 4, KernelPlacementPolicy::FlexHigh);
    a.dependencies.push_back("missing");
    library.addModule(a);
    library.addDispatch(0, "a_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x4000;

    KernelPackingResult result = KernelPacker::pack(request);
    CHECK(result.ok == false);
    CHECK(result.error.find("missing") != std::string::npos);
  }

  TEST_CASE("High-priority modules stay resident before low priority") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40E0, "test"});
    KernelModule high =
        makeModule("high", 10, KernelPlacementPolicy::FlexHigh);
    KernelModule low = makeModule("low", 17, KernelPlacementPolicy::FlexLow);
    library.addModule(high);
    library.addModule(low);
    library.addDispatch(0, "high_entry");
    library.addDispatch(1, "low_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.usedDispatch.insert(1);
    request.tableBaseAddress = 0x40E0;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);
    REQUIRE(result.placements.size() == 2);

    bool highResident = false;
    bool lowOverflow = false;
    for (std::size_t i = 0; i < result.placements.size(); i++) {
      if (result.placements[i].moduleId == "high") {
        highResident = result.placements[i].resident;
      }
      if (result.placements[i].moduleId == "low") {
        lowOverflow = !result.placements[i].resident;
      }
    }
    CHECK(highResident == true);
    CHECK(lowOverflow == true);
  }

  TEST_CASE("Game graphics stay resident before cold graphics") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40E0, "test"});
    KernelModule game =
        makeModule("game-graphics", 10, KernelPlacementPolicy::FlexHigh);
    KernelModule cold =
        makeModule("cold-graphics", 17, KernelPlacementPolicy::FlexLow);
    library.addModule(game);
    library.addModule(cold);
    library.addDispatch(0, "game-graphics_entry");
    library.addDispatch(1, "cold-graphics_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.usedDispatch.insert(1);
    request.tableBaseAddress = 0x40E0;
    request.romType = KernelRomType::MegaRom;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);

    bool gameResident = false;
    bool coldOverflow = false;
    for (std::size_t i = 0; i < result.placements.size(); i++) {
      if (result.placements[i].moduleId == "game-graphics") {
        gameResident = result.placements[i].resident;
      }
      if (result.placements[i].moduleId == "cold-graphics") {
        coldOverflow = !result.placements[i].resident;
      }
    }
    CHECK(gameResident == true);
    CHECK(coldOverflow == true);
  }

  TEST_CASE("Table-size iteration stabilizes with resident and overflow") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40E0, "test"});
    KernelModule a = makeModule("a", 10, KernelPlacementPolicy::FlexHigh);
    KernelModule b = makeModule("b", 10, KernelPlacementPolicy::FlexLow);
    KernelModule c = makeModule("c", 5, KernelPlacementPolicy::FlexHigh);
    library.addModule(a);
    library.addModule(b);
    library.addModule(c);
    library.addDispatch(0, "a_entry");
    library.addDispatch(1, "b_entry");
    library.addDispatch(2, "c_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.usedDispatch.insert(1);
    request.usedDispatch.insert(2);
    request.tableBaseAddress = 0x40E0;
    request.romType = KernelRomType::MegaRom;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);
    CHECK(result.tableSize == 12);

    bool bOverflow = false;
    for (std::size_t i = 0; i < result.placements.size(); i++) {
      if (result.placements[i].moduleId == "b") {
        bOverflow = !result.placements[i].resident;
      }
    }
    CHECK(bOverflow == true);
  }

  TEST_CASE("MegaROM overflow uses 8 KB-pair segment numbers") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40F0, "test"});
    KernelModule first =
        makeModule("first", 0x100, KernelPlacementPolicy::FlexLow);
    KernelModule second =
        makeModule("second", 0x100, KernelPlacementPolicy::FlexLow);
    library.addModule(first);
    library.addModule(second);
    library.addDispatch(0, "first_entry");
    library.addDispatch(1, "second_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.usedDispatch.insert(1);
    request.tableBaseAddress = 0x40F0;
    request.romType = KernelRomType::MegaRom;
    request.overflowStart = 0x8000;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);
    REQUIRE(result.dispatches.size() == 2);
    CHECK(result.dispatches[0].paged == true);
    CHECK(result.dispatches[1].paged == true);
    CHECK(result.dispatches[0].segment == 0);
    CHECK(result.dispatches[1].segment == 1);
    CHECK(result.dispatches[0].address == 0x8000);
    CHECK(result.dispatches[1].address == 0x8000);
  }

  TEST_CASE("Plain ROM budget failure suggests MegaROM") {
    InMemoryKernelModuleLibrary library;
    KernelModule big = makeModule("big", 0x4000, KernelPlacementPolicy::FlexLow);
    library.addModule(big);
    library.addDispatch(0, "big_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x4000;
    request.startCodeSize = 9000;
    request.userCodeSize = 8000;
    request.overflowStart = 0x8000;
    request.overflowCapacity = 0x4000;

    KernelPackingResult result = KernelPacker::pack(request);
    CHECK(result.ok == false);
    CHECK(result.error.find("Plain ROM") != std::string::npos);
    CHECK(result.error.find("MegaROM") != std::string::npos);
  }

  TEST_CASE("Overflow capacity failure names the module and available") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40F0, "test"});
    KernelModule big =
        makeModule("big", 20, KernelPlacementPolicy::FlexLow);
    library.addModule(big);
    library.addDispatch(0, "big_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x40F0;
    request.overflowCapacity = 10;

    KernelPackingResult result = KernelPacker::pack(request);
    CHECK(result.ok == false);
    CHECK(result.error.find("big") != std::string::npos);
    CHECK(result.error.find("available") != std::string::npos);
  }

  TEST_CASE("MegaROM overflow respects the mapper limit") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40F0, "test"});
    KernelModule a = makeModule("a", 0x100, KernelPlacementPolicy::FlexLow);
    library.addModule(a);
    library.addDispatch(0, "a_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x40F0;
    request.romType = KernelRomType::MegaRom;
    request.overflowStart = 0x8000;
    request.baseSegment = 254;
    request.maxSegments = 0;

    KernelPackingResult result = KernelPacker::pack(request);
    CHECK(result.ok == false);
    CHECK(result.error.find("mapper limit") != std::string::npos);
  }

  TEST_CASE("Game graphics overflow emits a diagnostic") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40F0, "test"});
    KernelModule game =
        makeModule("game-graphics", 20, KernelPlacementPolicy::FlexHigh);
    library.addModule(game);
    library.addDispatch(0, "game-graphics_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x40F0;

    KernelPackingResult result = KernelPacker::pack(request);
    REQUIRE(result.ok == true);
    bool found = false;
    for (std::size_t i = 0; i < result.diagnostics.size(); i++) {
      if (result.diagnostics[i].find("game graphics") != std::string::npos) {
        found = true;
      }
    }
    CHECK(found == true);
  }

  TEST_CASE("Pinned game graphics fails instead of overflowing") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40F0, "test"});
    KernelModule game =
        makeModule("game-graphics", 20, KernelPlacementPolicy::FlexHigh);
    library.addModule(game);
    library.addDispatch(0, "game-graphics_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.tableBaseAddress = 0x40F0;
    request.pinGameGraphics = true;

    KernelPackingResult result = KernelPacker::pack(request);
    CHECK(result.ok == false);
    CHECK(result.error.find("pinned") != std::string::npos);
  }
  TEST_CASE("Imported exports get their own central-table stub") {
    InMemoryKernelModuleLibrary library;
    KernelModule caller =
        makeModule("caller", 4, KernelPlacementPolicy::Fixed, 0x4000);
    KernelModule target =
        makeModule("target", 4, KernelPlacementPolicy::FlexLow);
    KernelImport imported;
    imported.target = "target_entry";
    imported.offset = 0;
    imported.kind = KernelImportKind::Address;
    caller.imports.push_back(imported);
    library.addModule(caller);
    library.addModule(target);

    KernelPackingRequest request = baseRequest(library);
    request.tableBaseAddress = 0x4040;
    request.kernelEnd = 0x4050;

    KernelPackingResult result = KernelPacker::pack(request);
    INFO(result.error);
    REQUIRE(result.ok == true);
    REQUIRE(result.imports.size() == 1);
    CHECK(result.imports[0].targetExport == "target_entry");
    CHECK(result.imports[0].stubAddress == 0x4040);
    REQUIRE(result.tableBytes.size() == 3);
    CHECK(result.tableBytes[0] == 0xC3);
  }

  TEST_CASE("Forced overflow of a fixed module is rejected") {
    InMemoryKernelModuleLibrary library;
    library.addModule(
        makeModule("fixed", 4, KernelPlacementPolicy::Fixed, 0x4000));

    KernelPackingRequest request = baseRequest(library);
    request.forceOverflowModules.insert("fixed");

    KernelPackingResult result = KernelPacker::pack(request);
    CHECK(result.ok == false);
    CHECK(result.error.find("fixed and cannot be relocated") !=
          std::string::npos);
  }

  TEST_CASE("Game graphics group is placed atomically") {
    InMemoryKernelModuleLibrary library;
    library.addFixedRegion({"reserved", 0x4000, 0x40E0, "test"});
    KernelModule first =
        makeModule("game-graphics-a", 10, KernelPlacementPolicy::FlexHigh);
    KernelModule second =
        makeModule("game-graphics-b", 10, KernelPlacementPolicy::FlexHigh);
    first.group = "game-graphics";
    second.group = "game-graphics";
    library.addModule(first);
    library.addModule(second);
    library.addDispatch(0, "game-graphics-a_entry");
    library.addDispatch(1, "game-graphics-b_entry");

    KernelPackingRequest request = baseRequest(library);
    request.usedDispatch.insert(0);
    request.usedDispatch.insert(1);
    request.tableBaseAddress = 0x40E0;
    request.kernelEnd = 0x40F0;

    KernelPackingResult result = KernelPacker::pack(request);
    INFO(result.error);
    REQUIRE(result.ok == true);
    bool firstResident = false;
    bool secondResident = false;
    for (std::size_t i = 0; i < result.placements.size(); i++) {
      if (result.placements[i].moduleId == "game-graphics-a") {
        firstResident = result.placements[i].resident;
      }
      if (result.placements[i].moduleId == "game-graphics-b") {
        secondResident = result.placements[i].resident;
      }
    }
    CHECK(firstResident == false);
    CHECK(secondResident == false);
  }

}

// NOLINTEND
