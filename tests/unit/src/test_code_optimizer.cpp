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
#include "compiler_hooks.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
#include "z80.h"

using namespace std;

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
}

// NOLINTEND
