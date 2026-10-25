/***
 * @file test_plain_overflow.cpp
 * @brief Plain ROM overflow module placement tests
 */

// NOLINTBEGIN

#include <memory>
#include <string>
#include <vector>

#include "compiler.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "kernel_module_library.h"
#include "rom.h"
#include "test_support.h"

TEST_SUITE("Plain ROM overflow") {
  TEST_CASE("Overflow module is copied after user code and called via a stub") {
    CompilationHarness harness;
    auto compiler =
        harness.compileSource("tmp/plain_overflow.bas", "10 CLS\n20 END\n");
    REQUIRE(compiler.get() != nullptr);

    auto library = std::make_shared<InMemoryKernelModuleLibrary>();

    KernelModule core;
    core.id = "core";
    core.name = "core";
    core.owner = "core";
    core.bytes.assign(0x3F00, 0x00);
    core.assemblyBase = 0x4000;
    core.fixedAddress = 0x4000;
    core.hasFixedAddress = true;
    core.policy = KernelPlacementPolicy::Fixed;
    library->addModule(core);

    KernelModule overflow;
    overflow.id = "overflow";
    overflow.name = "overflow";
    overflow.owner = "overflow";
    overflow.bytes.assign(0x0100, 0xAA);
    overflow.assemblyBase = 0;
    overflow.fixedAddress = 0;
    overflow.hasFixedAddress = false;
    overflow.policy = KernelPlacementPolicy::FlexHigh;
    KernelExport exportEntry;
    exportEntry.name = "overflow_entry";
    exportEntry.offset = 0;
    overflow.exports.push_back(exportEntry);
    library->addModule(overflow);

    const std::set<unsigned int>& used =
        compiler->getContext()->getUsedDispatch();
    for (std::set<unsigned int>::const_iterator it = used.begin();
         it != used.end(); ++it) {
      library->addDispatch(*it, "overflow_entry");
    }

    Rom rom(harness.romWriter, library);
    REQUIRE(rom.build(compiler) == true);

    std::vector<unsigned char> bytes = artifactBytes(*rom.getArtifact());
    unsigned int overflowAddress = 0x8000 +
        static_cast<unsigned int>(compiler->getKernelImage()->startSize()) +
        static_cast<unsigned int>(compiler->getCodeSize());
    std::size_t artifactOffset = 0x4000 + (overflowAddress - 0x8000);
    REQUIRE(artifactOffset + 0x0100 <= bytes.size());
    for (std::size_t i = 0; i < 0x0100; i++) {
      CHECK(bytes[artifactOffset + i] == 0xAA);
    }
  }
}

// NOLINTEND
