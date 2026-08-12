/***
 * @file test_compiler_on.cpp
 * @brief MSXBAS2ROM compiler ON statement unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "action_node.h"
#include "build_options.h"
#include "compiler.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_on_statement_strategy.h"
#include "compiler_statement_strategy_factory.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
#include "lexeme.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "z80.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static bool compileStatementProgram(const std::string& filename,
                                    const std::string& program,
                                    std::string* error_out = nullptr) {
  const std::string path = createTempBas(filename, program);

  shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter = make_shared<Z80OpcodeWriter>();
  shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  shared_ptr<Parser> parser = make_shared<Parser>();

  bool ok = false;
  if (lexer->load(path) && lexer->evaluate() && parser->evaluate(lexer)) {
    ok = compiler->build(parser);
  }

  if (!ok && error_out) {
    *error_out = compiler->getLogger()->errors().toString();
  }

  std::remove(path.c_str());

  return ok;
}

static shared_ptr<CompilerContext> createOnContext() {
  shared_ptr<CpuWorkspaceContext> workspace =
      make_shared<CpuWorkspaceContext>(COMPILE_CODE_SIZE, COMPILE_RAM_SIZE,
                                       def_RAM_BOTTOM);
  workspace->clear();
  shared_ptr<Z80OpcodeWriter> cpu = make_shared<Z80OpcodeWriter>();
  cpu->context = workspace;
  shared_ptr<CompilerContext> ctx = make_shared<CompilerContext>();
  ctx->setHelpers(ctx);
  ctx->cpu = cpu;
  ctx->opts = make_shared<BuildOptions>();
  ctx->parser = make_shared<Parser>();
  ctx->compiled = true;
  return ctx;
}

static shared_ptr<Lexeme> onNum(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_numeric, v);
}

static shared_ptr<Lexeme> onKw(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, v);
}

static shared_ptr<Lexeme> onId(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_numeric,
                             v);
}

static shared_ptr<ActionNode> onAction(const std::string& keyword) {
  return make_shared<ActionNode>(keyword);
}

TEST_SUITE("CompilerOnStatementStrategy") {
  TEST_CASE("ON trap statements compile successfully") {
    struct OnCase {
      const char* name;
      const char* program;
    };

    const OnCase cases[] = {
        {"ON_INTERVAL",
         "10 A=1\n20 ON INTERVAL=1 GOSUB 100\n30 END\n100 RETURN\n"},
        {"ON_KEY",
         "10 ON KEY GOSUB 100,200\n20 END\n100 RETURN\n200 RETURN\n"},
        {"ON_SPRITE", "10 ON SPRITE GOSUB 100\n20 END\n100 RETURN\n"},
        {"ON_STOP", "10 ON STOP GOSUB 100\n20 END\n100 RETURN\n"},
        {"ON_STRIG",
         "10 ON STRIG GOSUB 100,200\n20 END\n100 RETURN\n200 RETURN\n"},
        {"ON_INDEX_GOTO", "10 ON 1 GOTO 20,30\n20 END\n30 END\n"},
        {"ON_INDEX_GOSUB", "10 ON 1 GOSUB 20\n20 RETURN\n30 END\n"},
        {"ON_INDEX_GOTO_SINGLE", "10 ON 1 GOTO 20\n20 END\n"},
    };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        std::string errors;
        bool ok = compileStatementProgram(
            std::string("on_") + test_case.name + ".bas", test_case.program,
            &errors);
        CHECK(ok == true);
        CHECK(errors.empty());
      }
    }
  }

  TEST_CASE("ON statement with MegaROM option compiles") {
    SUBCASE("ON INTERVAL in MegaROM mode") {
      const std::string path =
          createTempBas("on_interval_mega.bas",
                        "10 A=1\n20 ON INTERVAL=1 GOSUB 100\n30 END\n"
                        "100 RETURN\n");

      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      opts->compileMode = BuildOptions::CompileMode::Konami4;
      opts->megaROM = true;
      opts->setInputFilename(path);

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      shared_ptr<Lexer> lexer = make_shared<Lexer>();
      shared_ptr<Parser> parser = make_shared<Parser>();

      bool ok = false;
      if (lexer->load(opts) && lexer->evaluate() && parser->evaluate(lexer)) {
        ok = compiler->build(parser);
      }

      CHECK(ok == true);
      std::remove(path.c_str());
    }

    SUBCASE("ON KEY with four handlers") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_key_four.bas",
          "10 ON KEY GOSUB 100,200,300,400\n20 END\n"
          "100 RETURN\n200 RETURN\n300 RETURN\n400 RETURN\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ON KEY with non-numeric handler uses dummy") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_key_dummy.bas",
          "10 ON KEY GOSUB 100,\"X\"\n20 END\n100 RETURN\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ON STRIG with four handlers") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_strig_four.bas",
          "10 ON STRIG GOSUB 100,200,300,400\n20 END\n"
          "100 RETURN\n200 RETURN\n300 RETURN\n400 RETURN\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ON STRIG with non-numeric handler uses dummy") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_strig_dummy.bas",
          "10 ON STRIG GOSUB 100,\"X\"\n20 END\n100 RETURN\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ON SPRITE with non-numeric handler uses dummy") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_sprite_dummy.bas", "10 ON SPRITE GOSUB \"X\"\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ON STOP with non-numeric handler uses dummy") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_stop_dummy.bas", "10 ON STOP GOSUB \"X\"\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ON KEY with handlers in MegaROM mode") {
      const std::string path = createTempBas(
          "on_key_four_mega.bas",
          "10 ON KEY GOSUB 100,200,300,400\n20 END\n"
          "100 RETURN\n200 RETURN\n300 RETURN\n400 RETURN\n");

      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      opts->compileMode = BuildOptions::CompileMode::Konami4;
      opts->megaROM = true;
      opts->setInputFilename(path);

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      shared_ptr<Lexer> lexer = make_shared<Lexer>();
      shared_ptr<Parser> parser = make_shared<Parser>();

      bool ok = false;
      if (lexer->load(opts) && lexer->evaluate() && parser->evaluate(lexer)) {
        ok = compiler->build(parser);
      }

      CHECK(ok == true);
      std::remove(path.c_str());
    }

    SUBCASE("ON STRIG with handlers in MegaROM mode") {
      const std::string path = createTempBas(
          "on_strig_four_mega.bas",
          "10 ON STRIG GOSUB 100,200,300,400\n20 END\n"
          "100 RETURN\n200 RETURN\n300 RETURN\n400 RETURN\n");

      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      opts->compileMode = BuildOptions::CompileMode::Konami4;
      opts->megaROM = true;
      opts->setInputFilename(path);

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      shared_ptr<Lexer> lexer = make_shared<Lexer>();
      shared_ptr<Parser> parser = make_shared<Parser>();

      bool ok = false;
      if (lexer->load(opts) && lexer->evaluate() && parser->evaluate(lexer)) {
        ok = compiler->build(parser);
      }

      CHECK(ok == true);
      std::remove(path.c_str());
    }

    SUBCASE("ON KEY in MegaROM mode") {
      const std::string path = createTempBas(
          "on_key_mega.bas",
          "10 ON KEY GOSUB 100,200\n20 END\n100 RETURN\n200 RETURN\n");

      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      opts->compileMode = BuildOptions::CompileMode::Konami4;
      opts->megaROM = true;
      opts->setInputFilename(path);

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      shared_ptr<Lexer> lexer = make_shared<Lexer>();
      shared_ptr<Parser> parser = make_shared<Parser>();

      bool ok = false;
      if (lexer->load(opts) && lexer->evaluate() && parser->evaluate(lexer)) {
        ok = compiler->build(parser);
      }

      CHECK(ok == true);
      std::remove(path.c_str());
    }

    SUBCASE("ON INDEX GOTO in MegaROM mode") {
      const std::string path = createTempBas(
          "on_index_mega.bas", "10 ON 1 GOTO 20,30\n20 END\n30 END\n");

      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      opts->compileMode = BuildOptions::CompileMode::Konami4;
      opts->megaROM = true;
      opts->setInputFilename(path);

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      shared_ptr<Lexer> lexer = make_shared<Lexer>();
      shared_ptr<Parser> parser = make_shared<Parser>();

      bool ok = false;
      if (lexer->load(opts) && lexer->evaluate() && parser->evaluate(lexer)) {
        ok = compiler->build(parser);
      }

      CHECK(ok == true);
      std::remove(path.c_str());
    }
  }

  TEST_CASE("ON error paths are detected") {
    SUBCASE("ON KEY missing GOSUB") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_err_key_gosub.bas", "10 ON KEY 100\n20 END\n", &errors);
      CHECK(ok == false);
    }

    SUBCASE("ON SPRITE missing GOSUB") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_err_sprite_gosub.bas", "10 ON SPRITE 100\n20 END\n", &errors);
      CHECK(ok == false);
    }

    SUBCASE("ON INTERVAL missing GOSUB") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_err_interval_gosub.bas",
          "10 ON INTERVAL=1 GOTO 100\n20 END\n", &errors);
      CHECK(ok == false);
    }

    SUBCASE("ON STOP missing GOSUB") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_err_stop_gosub.bas", "10 ON STOP 100\n20 END\n", &errors);
      CHECK(ok == false);
    }

    SUBCASE("ON SPRITE wrong parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "on_err_sprite_count.bas", "10 ON SPRITE GOSUB 100,200\n20 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.find("ON SPRITE with wrong count of parameters") !=
            std::string::npos);
    }
  }
}

TEST_SUITE("CompilerOnDirect") {
  TEST_CASE("ON ERROR reports not implemented") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    action->actions.push_back(make_shared<ActionNode>(onKw("ERROR")));

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find("Not implemented yet") !=
          std::string::npos);
  }

  TEST_CASE("Empty ON statement is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find("Empty ON statement") !=
          std::string::npos);
  }

  TEST_CASE("Invalid ON sub-command is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    action->actions.push_back(make_shared<ActionNode>(onKw("FOOBAR")));
    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find("Invalid ON statement") !=
          std::string::npos);
  }

  TEST_CASE("ON INTERVAL with missing INDEX is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> interval = make_shared<ActionNode>(onKw("INTERVAL"));
    interval->actions.push_back(make_shared<ActionNode>(onNum("1")));
    interval->actions.push_back(make_shared<ActionNode>(onKw("GOSUB")));
    action->actions.push_back(interval);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find(
              "Interval index is missing") != std::string::npos);
  }

  TEST_CASE("ON INTERVAL with wrong index count is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> interval = make_shared<ActionNode>(onKw("INTERVAL"));
    shared_ptr<ActionNode> index = make_shared<ActionNode>(onKw("INDEX"));
    index->actions.push_back(make_shared<ActionNode>(onNum("1")));
    index->actions.push_back(make_shared<ActionNode>(onNum("2")));
    interval->actions.push_back(index);
    interval->actions.push_back(make_shared<ActionNode>(onKw("GOSUB")));
    action->actions.push_back(interval);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find(
              "Wrong parameter count in interval index") !=
          std::string::npos);
  }

  TEST_CASE("ON INTERVAL with missing GOSUB is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> interval = make_shared<ActionNode>(onKw("INTERVAL"));
    shared_ptr<ActionNode> index = make_shared<ActionNode>(onKw("INDEX"));
    index->actions.push_back(make_shared<ActionNode>(onNum("1")));
    interval->actions.push_back(index);
    interval->actions.push_back(make_shared<ActionNode>(onKw("GOTO")));
    action->actions.push_back(interval);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find("GOSUB is missing") !=
          std::string::npos);
  }

  TEST_CASE("ON INTERVAL with empty parameters is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> interval = make_shared<ActionNode>(onKw("INTERVAL"));
    action->actions.push_back(interval);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find(
              "ON INTERVAL with empty parameters") != std::string::npos);
  }

  TEST_CASE("ON KEY with missing GOSUB is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> key = make_shared<ActionNode>(onKw("KEY"));
    key->actions.push_back(make_shared<ActionNode>(onNum("100")));
    action->actions.push_back(key);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find(
              "GOSUB parameters is missing") != std::string::npos);
  }

  TEST_CASE("ON SPRITE with missing GOSUB is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> sprite = make_shared<ActionNode>(onKw("SPRITE"));
    sprite->actions.push_back(make_shared<ActionNode>(onNum("100")));
    action->actions.push_back(sprite);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find(
              "GOSUB parameters is missing") != std::string::npos);
  }

  TEST_CASE("ON STOP with missing GOSUB is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    shared_ptr<ActionNode> stop = make_shared<ActionNode>(onKw("STOP"));
    stop->actions.push_back(make_shared<ActionNode>(onNum("100")));
    action->actions.push_back(stop);

    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find(
              "GOSUB parameters is missing") != std::string::npos);
  }

  TEST_CASE("ON with non-keyword sub-command is rejected") {
    CompilerOnStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createOnContext();

    shared_ptr<ActionNode> action = onAction("ON");
    action->actions.push_back(make_shared<ActionNode>(onNum("1")));
    ctx->current_action = action;
    strategy.execute(ctx);

    CHECK(ctx->logger->errors().toString().find("Invalid ON statement") !=
          std::string::npos);
  }
}

// NOLINTEND
