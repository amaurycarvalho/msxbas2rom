/***
 * @file test_code_optimizer.cpp
 * @brief MSXBAS2ROM compiler code optimizer (peephole) unit testing
 */

// NOLINTBEGIN

#include <memory>

#include "build_options.h"
#include "compiler.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
#include "fix_node.h"
#include "lexeme.h"
#include "symbol_node.h"
#include "tag_node.h"
#include "test_support.h"
#include "z80.h"

using namespace std;

extern unsigned char bin_header_bin[];

static shared_ptr<CompilerContext> makeOptimizerContext() {
  CompilationHarness harness;
  auto ctx = harness.makeContext();
  ctx->setHelpers(ctx);
  ctx->getOpts() = make_shared<BuildOptions>();
  auto workspace =
      make_shared<CpuWorkspaceContext>(COMPILE_CODE_SIZE, COMPILE_RAM_SIZE,
                                       def_RAM_BOTTOM);
  auto cpu = make_shared<Z80OpcodeWriter>();
  cpu->context = workspace;
  ctx->getCpu() = cpu;
  return ctx;
}

static shared_ptr<CompilerContext> makeMegaRomContext() {
  auto ctx = makeOptimizerContext();
  ctx->getOpts()->megaROM = true;
  return ctx;
}

TEST_SUITE("CompilerCodeOptimizer") {
  TEST_CASE("Optimizes ld hl | push hl | ld hl | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addLdHL(0x1234);
    cpu.addPushHL();
    cpu.addLdHL(0x5678);
    ctx->getCodeOptimizer()->addByteOptimized(0xD1);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x11);  // ld de
    CHECK(code[1] == 0x34);
    CHECK(code[2] == 0x12);
    CHECK(code[3] == 0x21);  // ld hl
    CHECK(code[4] == 0x78);
    CHECK(code[5] == 0x56);
    CHECK(ctx->getCpu()->context->getCodeSize() == 6);
  }

  TEST_CASE("Optimizes push hl | ld hl | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addPushHL();
    cpu.addLdHL(0x1234);
    ctx->getCodeOptimizer()->addByteOptimized(0xD1);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0xEB);  // ex de,hl
    CHECK(code[1] == 0x21);  // ld hl
  }

  TEST_CASE("Optimizes add hl,de with immediate one") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addExDEHL();
    cpu.addLdHL(1);
    ctx->getCodeOptimizer()->addByteOptimized(0x19);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x23);  // inc hl
  }

  TEST_CASE("Optimizes add hl,de with immediate two") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addExDEHL();
    cpu.addLdHL(2);
    ctx->getCodeOptimizer()->addByteOptimized(0x19);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x23);
    CHECK(code[1] == 0x23);
  }

  TEST_CASE("Optimizes add hl,de with immediate three") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addExDEHL();
    cpu.addLdHL(3);
    ctx->getCodeOptimizer()->addByteOptimized(0x19);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x23);
    CHECK(code[1] == 0x23);
    CHECK(code[2] == 0x23);
  }

  TEST_CASE("Optimizes add hl,de with large immediate to ld de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addExDEHL();
    cpu.addLdHL(0x1234);
    ctx->getCodeOptimizer()->addByteOptimized(0x19);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x11);  // ld de
    CHECK(code[3] == 0x19);  // add hl,de
  }

  TEST_CASE("Optimizes ex de,hl | ld hl | ex de,hl to ld de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addExDEHL();
    cpu.addLdHL(0x1234);
    ctx->getCodeOptimizer()->addByteOptimized(0xEB);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x11);  // ld de
    CHECK(code[1] == 0x34);
    CHECK(code[2] == 0x12);
  }

  TEST_CASE("Leaves unmatched sequence untouched") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addPushHL();
    ctx->getCodeOptimizer()->addByteOptimized(0xD1);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0xE5);  // push hl unchanged
    CHECK(code[1] == 0xD1);  // pop de appended
  }

  TEST_CASE("Optimizes ld hl,(x) | push hl | ld hl,(x) | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addLdHLii(0x1234);
    cpu.addPushHL();
    cpu.addLdHLii(0x5678);
    ctx->getCodeOptimizer()->addByteOptimized(0xD1);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x2A);  // ld hl,(x)
    CHECK(code[3] == 0xEB);  // ex de,hl replaces push hl
    CHECK(code[4] == 0x2A);  // ld hl,(x)
  }

  TEST_CASE("Optimizes push hl | ld hl,(x) | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addPushHL();
    cpu.addLdHLii(0x1234);
    ctx->getCodeOptimizer()->addByteOptimized(0xD1);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0xEB);  // ex de,hl replaces push hl
    CHECK(code[1] == 0x2A);  // ld hl,(x)
  }

  TEST_CASE("Optimizes ld de,x | ld hl,x | ex de,hl") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addLdDE(0x1234);
    cpu.addLdHL(0x5678);
    ctx->getCodeOptimizer()->addByteOptimized(0xEB);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x21);  // ld hl,x
    CHECK(code[3] == 0x11);  // ld de,x
  }

  TEST_CASE("Optimizes ld hl,x | ld de,x | ex de,hl") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addLdHL(0x5678);
    cpu.addLdDE(0x1234);
    ctx->getCodeOptimizer()->addByteOptimized(0xEB);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0x11);  // ld de,x
    CHECK(code[3] == 0x21);  // ld hl,x
  }

  TEST_CASE("getKernelCallAddr resolves and passes through boundaries") {
    auto ctx = makeOptimizerContext();

    // in the wrapper dispatch table -> resolves from bin_header_bin
    int resolved = ctx->getCodeOptimizer()->getKernelCallAddr(0);
    CHECK(resolved == (bin_header_bin[0] | (bin_header_bin[1] << 8)));

    // past the table but below 0x4000 -> unchanged
    int past = ctx->getCodeOptimizer()->getKernelCallAddr(
        def_wrapper_routines_map_table + DISP_ENTRIES * 2);
    CHECK(past == def_wrapper_routines_map_table + DISP_ENTRIES * 2);

    // at/above 0x8000 -> unchanged
    CHECK(ctx->getCodeOptimizer()->getKernelCallAddr(0x8000) == 0x8000);
  }

  TEST_CASE("addKernelCall and addKernelCallNZ emit a call") {
    auto ctx = makeOptimizerContext();
    ctx->getCodeOptimizer()->addKernelCall(0);
    ctx->getCodeOptimizer()->addKernelCallNZ(0);

    auto& code = ctx->getCpu()->context->getCode();
    CHECK(code[0] == 0xCD);  // call
    CHECK(code[3] == 0xC4);  // call nz
  }

  TEST_CASE("Pop-de optimization shifts subsequent fix addresses") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->getCpu();
    cpu.addLdHL(0x1234);
    cpu.addPushHL();
    cpu.addLdHL(0x5678);

    auto sym = make_shared<SymbolNode>();
    sym->lexeme = make_shared<Lexeme>(Lexeme::type_keyword,
                                      Lexeme::subtype_any, "100");
    sym->address = 0x1000;
    auto fix = make_shared<FixNode>();
    fix->symbol = sym;
    fix->address = cpu.context->getCodePointer();  // after the three instructions
    fix->step = 0;
    ctx->getFixes().push_back(fix);

    ctx->getCodeOptimizer()->addByteOptimized(0xD1);  // pop de -> reduces code by 1

    CHECK(fix->address == cpu.context->getCodePointer());  // shifted down by 1
    CHECK(ctx->getCpu()->context->getCodeSize() == 6);
  }
}

TEST_SUITE("CompilerFixupResolver") {
  TEST_CASE("addFix skips trampoline for identifiers in MegaROM") {
    auto ctx = makeMegaRomContext();
    auto& fixup = *ctx->getFixupResolver();
    auto& cpu = *ctx->getCpu();

    auto idSym = make_shared<SymbolNode>();
    idSym->lexeme = make_shared<Lexeme>(Lexeme::type_identifier,
                                        Lexeme::subtype_numeric, "A", "A");
    int before = cpu.context->getCodeSize();
    fixup.addFix(idSym);
    int idEmitted = cpu.context->getCodeSize() - before;

    auto lineSym = make_shared<SymbolNode>();
    lineSym->lexeme = make_shared<Lexeme>(Lexeme::type_keyword,
                                          Lexeme::subtype_any, "100", "100");
    before = cpu.context->getCodeSize();
    fixup.addFix(lineSym);
    int lineEmitted = cpu.context->getCodeSize() - before;

    CHECK(idEmitted == 0);
    CHECK(lineEmitted > 0);
    CHECK(lineEmitted > idEmitted);
  }

  TEST_CASE("addPreMark produces sequential mark names") {
    auto ctx = makeMegaRomContext();
    auto& fixup = *ctx->getFixupResolver();

    auto mark0 = fixup.addPreMark();
    auto mark1 = fixup.addPreMark();

    REQUIRE(mark0.get() != nullptr);
    REQUIRE(mark1.get() != nullptr);
    REQUIRE(mark0->tag.get() != nullptr);
    REQUIRE(mark1->tag.get() != nullptr);
    CHECK(mark0->tag->name == "MARK_0");
    CHECK(mark1->tag->name == "MARK_1");
  }

  TEST_CASE("doFix writes the 16-bit resolved address") {
    auto ctx = makeMegaRomContext();
    auto& fixup = *ctx->getFixupResolver();
    auto& cpu = *ctx->getCpu();

    auto sym = make_shared<SymbolNode>();
    sym->lexeme = make_shared<Lexeme>(Lexeme::type_identifier,
                                      Lexeme::subtype_numeric, "A", "A");
    sym->address = 0x1234;

    auto fix = fixup.addFix(sym);
    REQUIRE(fix.get() != nullptr);

    fixup.doFix();

    CHECK(cpu.context->getCode()[fix->address] == 0x34);
    CHECK(cpu.context->getCode()[fix->address + 1] == 0x12);
  }
}

// NOLINTEND
