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
#include "z80.h"

using namespace std;

extern unsigned char bin_header_bin[];

static shared_ptr<CompilerContext> makeOptimizerContext() {
  auto ctx = make_shared<CompilerContext>();
  ctx->setHelpers(ctx);
  ctx->opts = make_shared<BuildOptions>();
  auto workspace =
      make_shared<CpuWorkspaceContext>(COMPILE_CODE_SIZE, COMPILE_RAM_SIZE,
                                       def_RAM_BOTTOM);
  auto cpu = make_shared<Z80OpcodeWriter>();
  cpu->context = workspace;
  ctx->cpu = cpu;
  return ctx;
}

static shared_ptr<CompilerContext> makeMegaRomContext() {
  auto ctx = makeOptimizerContext();
  ctx->opts->megaROM = true;
  return ctx;
}

TEST_SUITE("CompilerCodeOptimizer") {
  TEST_CASE("Optimizes ld hl | push hl | ld hl | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addLdHL(0x1234);
    cpu.addPushHL();
    cpu.addLdHL(0x5678);
    ctx->codeOptimizer->addByteOptimized(0xD1);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x11);  // ld de
    CHECK(code[1] == 0x34);
    CHECK(code[2] == 0x12);
    CHECK(code[3] == 0x21);  // ld hl
    CHECK(code[4] == 0x78);
    CHECK(code[5] == 0x56);
    CHECK(ctx->cpu->context->code_size == 6);
  }

  TEST_CASE("Optimizes push hl | ld hl | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addPushHL();
    cpu.addLdHL(0x1234);
    ctx->codeOptimizer->addByteOptimized(0xD1);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0xEB);  // ex de,hl
    CHECK(code[1] == 0x21);  // ld hl
  }

  TEST_CASE("Optimizes add hl,de with immediate one") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addExDEHL();
    cpu.addLdHL(1);
    ctx->codeOptimizer->addByteOptimized(0x19);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x23);  // inc hl
  }

  TEST_CASE("Optimizes add hl,de with immediate two") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addExDEHL();
    cpu.addLdHL(2);
    ctx->codeOptimizer->addByteOptimized(0x19);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x23);
    CHECK(code[1] == 0x23);
  }

  TEST_CASE("Optimizes add hl,de with immediate three") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addExDEHL();
    cpu.addLdHL(3);
    ctx->codeOptimizer->addByteOptimized(0x19);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x23);
    CHECK(code[1] == 0x23);
    CHECK(code[2] == 0x23);
  }

  TEST_CASE("Optimizes add hl,de with large immediate to ld de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addExDEHL();
    cpu.addLdHL(0x1234);
    ctx->codeOptimizer->addByteOptimized(0x19);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x11);  // ld de
    CHECK(code[3] == 0x19);  // add hl,de
  }

  TEST_CASE("Optimizes ex de,hl | ld hl | ex de,hl to ld de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addExDEHL();
    cpu.addLdHL(0x1234);
    ctx->codeOptimizer->addByteOptimized(0xEB);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x11);  // ld de
    CHECK(code[1] == 0x34);
    CHECK(code[2] == 0x12);
  }

  TEST_CASE("Leaves unmatched sequence untouched") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addPushHL();
    ctx->codeOptimizer->addByteOptimized(0xD1);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0xE5);  // push hl unchanged
    CHECK(code[1] == 0xD1);  // pop de appended
  }

  TEST_CASE("Optimizes ld hl,(x) | push hl | ld hl,(x) | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addLdHLii(0x1234);
    cpu.addPushHL();
    cpu.addLdHLii(0x5678);
    ctx->codeOptimizer->addByteOptimized(0xD1);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x2A);  // ld hl,(x)
    CHECK(code[3] == 0xEB);  // ex de,hl replaces push hl
    CHECK(code[4] == 0x2A);  // ld hl,(x)
  }

  TEST_CASE("Optimizes push hl | ld hl,(x) | pop de") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addPushHL();
    cpu.addLdHLii(0x1234);
    ctx->codeOptimizer->addByteOptimized(0xD1);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0xEB);  // ex de,hl replaces push hl
    CHECK(code[1] == 0x2A);  // ld hl,(x)
  }

  TEST_CASE("Optimizes ld de,x | ld hl,x | ex de,hl") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addLdDE(0x1234);
    cpu.addLdHL(0x5678);
    ctx->codeOptimizer->addByteOptimized(0xEB);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x21);  // ld hl,x
    CHECK(code[3] == 0x11);  // ld de,x
  }

  TEST_CASE("Optimizes ld hl,x | ld de,x | ex de,hl") {
    auto ctx = makeOptimizerContext();
    auto& cpu = *ctx->cpu;
    cpu.addLdHL(0x5678);
    cpu.addLdDE(0x1234);
    ctx->codeOptimizer->addByteOptimized(0xEB);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0x11);  // ld de,x
    CHECK(code[3] == 0x21);  // ld hl,x
  }

  TEST_CASE("getKernelCallAddr resolves and passes through boundaries") {
    auto ctx = makeOptimizerContext();

    // in the wrapper dispatch table -> resolves from bin_header_bin
    int resolved = ctx->codeOptimizer->getKernelCallAddr(0);
    CHECK(resolved == (bin_header_bin[0] | (bin_header_bin[1] << 8)));

    // past the table but below 0x4000 -> unchanged
    int past = ctx->codeOptimizer->getKernelCallAddr(
        def_wrapper_routines_map_table + DISP_ENTRIES * 2);
    CHECK(past == def_wrapper_routines_map_table + DISP_ENTRIES * 2);

    // at/above 0x8000 -> unchanged
    CHECK(ctx->codeOptimizer->getKernelCallAddr(0x8000) == 0x8000);
  }

  TEST_CASE("addKernelCall and addKernelCallNZ emit a call") {
    auto ctx = makeOptimizerContext();
    ctx->codeOptimizer->addKernelCall(0);
    ctx->codeOptimizer->addKernelCallNZ(0);

    auto& code = ctx->cpu->context->code;
    CHECK(code[0] == 0xCD);  // call
    CHECK(code[3] == 0xC4);  // call nz
  }
}

TEST_SUITE("CompilerFixupResolver") {
  TEST_CASE("addFix skips trampoline for identifiers in MegaROM") {
    auto ctx = makeMegaRomContext();
    auto& fixup = *ctx->fixupResolver;
    auto& cpu = *ctx->cpu;

    auto idSym = make_shared<SymbolNode>();
    idSym->lexeme = make_shared<Lexeme>(Lexeme::type_identifier,
                                        Lexeme::subtype_numeric, "A", "A");
    int before = cpu.context->code_size;
    fixup.addFix(idSym);
    int idEmitted = cpu.context->code_size - before;

    auto lineSym = make_shared<SymbolNode>();
    lineSym->lexeme = make_shared<Lexeme>(Lexeme::type_keyword,
                                          Lexeme::subtype_any, "100", "100");
    before = cpu.context->code_size;
    fixup.addFix(lineSym);
    int lineEmitted = cpu.context->code_size - before;

    CHECK(idEmitted == 0);
    CHECK(lineEmitted > 0);
    CHECK(lineEmitted > idEmitted);
  }

  TEST_CASE("addPreMark produces sequential mark names") {
    auto ctx = makeMegaRomContext();
    auto& fixup = *ctx->fixupResolver;

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
    auto& fixup = *ctx->fixupResolver;
    auto& cpu = *ctx->cpu;

    auto sym = make_shared<SymbolNode>();
    sym->lexeme = make_shared<Lexeme>(Lexeme::type_identifier,
                                      Lexeme::subtype_numeric, "A", "A");
    sym->address = 0x1234;

    auto fix = fixup.addFix(sym);
    REQUIRE(fix.get() != nullptr);

    fixup.doFix();

    CHECK(cpu.context->code[fix->address] == 0x34);
    CHECK(cpu.context->code[fix->address + 1] == 0x12);
  }
}

// NOLINTEND
