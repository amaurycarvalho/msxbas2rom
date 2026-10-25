/***
 * @file test_generated_module_library.cpp
 * @brief Generated module library and resident composition tests
 */

// NOLINTBEGIN

#include <map>
#include <set>
#include <string>
#include <vector>

#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "generated_kernel_module_library.h"
#include "kernel_dispatch_generated.h"
#include "kernel_manifest_generated.h"
#include "kernel_packer.h"
#include "rom.h"
#include "test_support.h"

TEST_SUITE("Generated module library") {
  TEST_CASE("Generated library exposes modules and dispatch exports") {
    GeneratedKernelModuleLibrary library;

    CHECK(library.modules().size() >= KERNEL_MANIFEST_MODULE_COUNT);
    CHECK(library.dispatchExports().size() == KERNEL_DISPATCH_COUNT);
    CHECK(std::string(kernel_dispatch_names[DISP_XBASIC_CLS]) ==
          "XBASIC_CLS");
    CHECK(library.fixedRegions().size() ==
          KERNEL_MANIFEST_FIXED_REGION_COUNT);

    for (std::size_t i = 0; i < library.modules().size(); i++) {
      const KernelModule& module = library.modules()[i];
      CHECK(module.bytes.size() > 0);
      CHECK(module.group.empty() == false);
      CHECK(module.classification.empty() == false);
    }

    bool hasDispatcher = false;
    for (std::size_t i = 0; i < library.modules().size(); i++) {
      if (library.modules()[i].id == "dispatcher") hasDispatcher = true;
    }
    CHECK(hasDispatcher == true);
  }

  TEST_CASE("Builder places the central table and patches call stubs") {
    CompilationHarness harness;
    auto compiler =
        harness.compileSource("tmp/composed_rom.bas", "10 CLS\n20 END\n");
    REQUIRE(compiler.get() != nullptr);

    auto library = std::make_shared<GeneratedKernelModuleLibrary>();
    Rom rom(harness.romWriter, library);
    REQUIRE(rom.build(compiler) == true);

    KernelPackingRequest request;
    request.library = library.get();
    request.usedDispatch = compiler->getContext()->getUsedDispatch();
    request.kernelStart = 0x4000;
    request.kernelEnd = 0x8000;
    request.tableBaseAddress = 0;
    request.dispatcherAddress = 0x4000;
    request.romType = KernelRomType::Plain;
    request.overflowStart = 0x8000;
    request.overflowCapacity = 0x4000;
    request.startCodeSize = static_cast<unsigned int>(
        compiler->getKernelImage()->startSize());
    request.userCodeSize =
        static_cast<unsigned int>(compiler->getCodeSize());

    KernelPackingResult packing = KernelPacker::pack(request);
    INFO(packing.error);
    REQUIRE(packing.ok == true);

    std::vector<unsigned char> bytes = artifactBytes(*rom.getArtifact());
    for (std::size_t i = 0; i < packing.tableBytes.size(); i++) {
      CHECK(bytes[packing.tableBaseAddress - 0x4000 + i] ==
            packing.tableBytes[i]);
    }

    unsigned int stubAddress = 0;
    for (std::size_t i = 0; i < packing.dispatches.size(); i++) {
      if (packing.dispatches[i].dispatchIndex == DISP_XBASIC_CLS) {
        stubAddress = packing.dispatches[i].stubAddress;
      }
    }
    REQUIRE(stubAddress != 0);

    const auto& fixups = compiler->getContext()->getDispatchFixups();
    bool checked = false;
    for (std::size_t i = 0; i < fixups.size(); i++) {
      if (fixups[i].dispatchIndex != DISP_XBASIC_CLS) continue;
      std::size_t offset = 0x4000 +
          compiler->getKernelImage()->startSize() + fixups[i].address;
      CHECK(bytes[offset] == (stubAddress & 0xFF));
      CHECK(bytes[offset + 1] == ((stubAddress >> 8) & 0xFF));
      checked = true;
      break;
    }
    CHECK(checked == true);
  }

  TEST_CASE("Relocatable module override is not pinned") {
    GeneratedKernelModuleLibrary library;

    const KernelModule* tileSprite = nullptr;
    for (std::size_t i = 0; i < library.modules().size(); i++) {
      if (library.modules()[i].id == "31-cmd-tile-sprite") {
        tileSprite = &library.modules()[i];
      }
    }
    REQUIRE(tileSprite != nullptr);
    CHECK(tileSprite->hasFixedAddress == false);
    CHECK(tileSprite->assemblyBase == 0);
    CHECK(tileSprite->relocations.size() > 0);
  }

  TEST_CASE("Generated library exposes module imports") {
    GeneratedKernelModuleLibrary library;

    const KernelModule* basic = nullptr;
    const KernelModule* math = nullptr;
    for (std::size_t i = 0; i < library.modules().size(); i++) {
      if (library.modules()[i].id == "30_basic_helpers") {
        basic = &library.modules()[i];
      }
      if (library.modules()[i].id == "90-math") {
        math = &library.modules()[i];
      }
    }
    REQUIRE(basic != nullptr);
    REQUIRE(math != nullptr);
    CHECK(basic->imports.size() > 0);
    CHECK(math->hasFixedAddress == false);

    bool hasString = false;
    bool hasUsr = false;
    for (std::size_t i = 0; i < basic->imports.size(); i++) {
      if (basic->imports[i].target == "C7EF2") hasString = true;
      if (basic->imports[i].target == "usr0") hasUsr = true;
    }
    CHECK(hasString == true);
    CHECK(hasUsr == false);
  }

  TEST_CASE("Page-unsafe resource modules remain fixed") {
    GeneratedKernelModuleLibrary library;

    for (std::size_t i = 0; i < library.modules().size(); i++) {
      const KernelModule& module = library.modules()[i];
      if (module.id == "32_usr" || module.id == "70_pletter" ||
          module.id == "31-cmd-mtf" ||
          module.id == "31-cmd-game-graphics" ||
          module.id == "31-cmd-cold-graphics" ||
          module.id == "90-graphics") {
        CHECK(module.hasFixedAddress == true);
        CHECK(module.policy == KernelPlacementPolicy::Fixed);
      }
    }
  }

  TEST_CASE("Packer composes a resident kernel from the used closure") {
    GeneratedKernelModuleLibrary library;

    KernelPackingRequest request;
    request.library = &library;
    request.usedDispatch.insert(DISP_XBASIC_CLS);
    request.kernelStart = 0x4000;
    request.kernelEnd = 0x8000;
    request.tableBaseAddress = 0x4A86;
    request.dispatcherAddress = 0x4A86;
    request.romType = KernelRomType::Plain;
    request.overflowStart = 0x8000;
    request.overflowCapacity = 0x4000;

    KernelPackingResult packed = KernelPacker::pack(request);
    INFO(packed.error);
    REQUIRE(packed.ok == true);
    CHECK(packed.tableBaseAddress == 0x4A86);
    CHECK(packed.tableBytes.size() == packed.tableSize);

    bool hasTileSprite = false;
    for (std::size_t i = 0; i < packed.placements.size(); i++) {
      if (packed.placements[i].moduleId == "31-cmd-tile-sprite") {
        hasTileSprite = true;
      }
    }
    CHECK(hasTileSprite == false);

    KernelCompositionResult composed =
        KernelComposer::compose(library, packed, 0x4000, 0x8000);
    INFO(composed.error);
    REQUIRE(composed.ok == true);
    CHECK(composed.image.size() == 0x4000);

    for (std::size_t i = 0; i < packed.tableBytes.size(); i++) {
      CHECK(composed.image[packed.tableBaseAddress - 0x4000 + i] ==
            packed.tableBytes[i]);
    }

    const KernelModule* basic = nullptr;
    for (std::size_t i = 0; i < library.modules().size(); i++) {
      if (library.modules()[i].id == "30_basic_helpers") {
        basic = &library.modules()[i];
      }
    }
    REQUIRE(basic != nullptr);
    std::map<std::string, unsigned int> importStubs;
    for (std::size_t i = 0; i < packed.imports.size(); i++) {
      importStubs[packed.imports[i].targetExport] =
          packed.imports[i].stubAddress;
    }
    std::set<unsigned int> importOffsets;
    for (std::size_t i = 0; i < basic->imports.size(); i++) {
      importOffsets.insert(basic->imports[i].offset);
      importOffsets.insert(basic->imports[i].offset + 1);
    }
    for (std::size_t i = 0; i < basic->bytes.size(); i++) {
      if (importOffsets.count(static_cast<unsigned int>(i))) {
        continue;
      }
      INFO("basic offset ", i);
      CHECK(composed.image[basic->assemblyBase - 0x4000 + i] ==
            basic->bytes[i]);
    }
    for (std::size_t i = 0; i < basic->imports.size(); i++) {
      const KernelImport& entry = basic->imports[i];
      REQUIRE(importStubs.count(entry.target) == 1);
      unsigned int stub = importStubs[entry.target];
      std::size_t offset = basic->assemblyBase - 0x4000 + entry.offset;
      CHECK(composed.image[offset] == (stub & 0xFF));
      CHECK(composed.image[offset + 1] == ((stub >> 8) & 0xFF));
    }
  }
}

// NOLINTEND
