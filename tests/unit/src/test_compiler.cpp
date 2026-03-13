/***
 * @file test_compiler.cpp
 * @brief MSXBAS2ROM compiler unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "compiler.h"
#include "compiler_cmd_handler_factory.h"
#include "compiler_function_strategy_factory.h"
#include "compiler_statement_strategy_factory.h"
#include "doctest/doctest.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "resources.h"
#include "z80.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static bool compileProgram(const std::string& filename, Compiler& compiler,
                           Parser& parser) {
  Lexer lexer;
  if (!lexer.load(filename)) return false;
  if (!lexer.evaluate()) return false;
  if (!parser.evaluate(&lexer)) return false;
  return compiler.build(&parser);
}

static bool compileProgram(const std::string& filename, Compiler& compiler) {
  Lexer lexer;
  Parser parser;

  if (!lexer.load(filename)) return false;
  if (!lexer.evaluate()) return false;
  if (!parser.evaluate(&lexer)) return false;
  return compiler.build(&parser);
}

TEST_SUITE("Compiler") {
  TEST_CASE("Compiles valid parser output") {
    const std::string filename =
        createTempBas("compiler_valid.bas", "10 PRINT \"HI\"\n20 END\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == true);
    CHECK(compiler.isCompiled() == true);
    CHECK(compiler.getCodeSize() > 0);
    CHECK(compiler.getRamSize() >= 0);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when parser has no tags") {
    const std::string filename = createTempBas("compiler_empty.bas", "\n\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.isCompiled() == false);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails on duplicated line numbers") {
    const std::string filename = createTempBas(
        "compiler_duplicated_line.bas", "10 PRINT \"A\"\n10 PRINT \"B\"\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.getLogger()->errors().toString().find(
              "Line number already declared") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when FOR has no NEXT") {
    const std::string filename = createTempBas(
        "compiler_for_without_next.bas", "10 FOR I=1 TO 10\n20 PRINT I\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.getLogger()->errors().toString().find(
              "FOR without a NEXT") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when NEXT has no FOR") {
    const std::string filename =
        createTempBas("compiler_next_without_for.bas", "10 NEXT I\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.getLogger()->errors().toString().find(
              "NEXT without a FOR") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails on unknown function or array in expression") {
    const std::string filename =
        createTempBas("compiler_unknown_func.bas", "10 A=FOO(1)\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.getLogger()->errors().toString().find(
              "Undeclared array or unknown function") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Registers DATA and IDATA resources") {
    const std::string filename =
        createTempBas("compiler_data_idata.bas", "10 DATA 1,2\n20 IDATA 3\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    Parser parser;
    CHECK(compileProgram(filename, compiler, parser) == true);
    CHECK(compiler.getResourceManager() != nullptr);
    CHECK(compiler.getResourceManager()->resources.size() >= 2);

    std::remove(filename.c_str());
  }

  TEST_CASE("Writes compiled code to output buffer") {
    const std::string filename =
        createTempBas("compiler_write.bas", "10 PRINT \"HI\"\n20 END\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == true);

    std::vector<unsigned char> out(0x8000, 0);
    int written = compiler.write(out.data(), 0x8000);
    CHECK(written > 0);

    std::remove(filename.c_str());
  }
}

TEST_SUITE("CompilerCmdHandlerFactory") {
  TEST_CASE("All CMD handlers have unit tests") {
    CompilerCmdHandlerFactory factory;
    CHECK(factory.size() == 42);
  }

  TEST_CASE("All CMD handlers are registered") {
    CompilerCmdHandlerFactory factory;

    std::vector<std::string> keywords = {
        "RUNASM",    "RUNBAS",    "WRTVRAM",   "WRTFNT",    "WRTCHR",
        "WRTCLR",    "WRTSCR",    "WRTSPR",    "WRTSPRPAT", "WRTSPRCLR",
        "WRTSPRATR", "RAMTOVRAM", "VRAMTORAM", "RAMTORAM",  "RSCTORAM",
        "DISSCR",    "ENASCR",    "KEYCLKOFF", "MUTE",      "PLAY",
        "DRAW",      "CLIP",      "PT3LOAD",   "PT3PLAY",   "PT3MUTE",
        "PT3LOOP",   "PT3REPLAY", "PLYLOAD",   "PLYPLAY",   "PLYSONG",
        "PLYMUTE",   "PLYLOOP",   "PLYREPLAY", "PLYSOUND",  "MTF",
        "SETFNT",    "UPDFNTCLR", "CLRSCR",    "CLRKEY",    "PAGE",
        "RESTORE",   "TURBO"};

    for (const auto& kw : keywords) {
      CHECK(factory.getByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    CompilerCmdHandlerFactory factory;

    CHECK(factory.getByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getByKeyword("") == nullptr);
    CHECK(factory.getByKeyword("INVALIDCMD") == nullptr);
  }

  TEST_CASE("PT3 commands share same handler") {
    CompilerCmdHandlerFactory factory;

    auto* h1 = factory.getByKeyword("PT3LOAD");
    auto* h2 = factory.getByKeyword("PT3PLAY");
    auto* h3 = factory.getByKeyword("PT3MUTE");
    auto* h4 = factory.getByKeyword("PT3LOOP");
    auto* h5 = factory.getByKeyword("PT3REPLAY");

    CHECK(h1 != nullptr);
    CHECK(h1 == h2);
    CHECK(h1 == h3);
    CHECK(h1 == h4);
    CHECK(h1 == h5);
  }

  TEST_CASE("PLY commands have independent handlers") {
    CompilerCmdHandlerFactory factory;

    CHECK(factory.getByKeyword("PLYLOAD") != nullptr);
    CHECK(factory.getByKeyword("PLYPLAY") != nullptr);
    CHECK(factory.getByKeyword("PLYSONG") != nullptr);
    CHECK(factory.getByKeyword("PLYMUTE") != nullptr);
    CHECK(factory.getByKeyword("PLYLOOP") != nullptr);
    CHECK(factory.getByKeyword("PLYREPLAY") != nullptr);
    CHECK(factory.getByKeyword("PLYSOUND") != nullptr);
  }

  TEST_CASE("Repeated calls return same handler instance") {
    CompilerCmdHandlerFactory factory;

    auto* h1 = factory.getByKeyword("RUNASM");
    auto* h2 = factory.getByKeyword("RUNASM");

    CHECK(h1 != nullptr);
    CHECK(h1 == h2);
  }
}

TEST_SUITE("CompilerStatementStrategyFactory") {
  TEST_CASE("All strategies are registered") {
    CompilerStatementStrategyFactory factory;

    std::vector<std::string> keywords = {

        "REM",      "'",

        "FILE",     "TEXT",

        "CLEAR",    "DEF",       "DEFINT",   "DEFSNG", "DEFDBL", "DEFSTR",
        "DEFUSR",   "CLS",       "BEEP",     "LET",    "TIME",   "DIM",
        "REDIM",    "RANDOMIZE",

        "END",      "GOTO",      "GOSUB",    "RETURN", "IF",     "FOR",
        "NEXT",     "ON",        "INTERVAL", "STOP",

        "PRINT",    "INPUT",     "SOUND",    "OUT",    "POKE",   "VPOKE",
        "IPOKE",    "DATA",      "IDATA",    "READ",   "IREAD",  "RESTORE",
        "IRESTORE", "RESUME",    "WAIT",     "SWAP",   "CALL",   "CMD",
        "MAXFILES", "OPEN",      "OPEN_GRP", "CLOSE",  "BLOAD",

        "PLAY",     "DRAW",      "LOCATE",   "SCREEN", "WIDTH",  "COLOR",
        "PSET",     "PRESET",    "LINE",     "PAINT",  "CIRCLE", "COPY",
        "PUT",      "GET",       "SET",      "KEY",    "STRIG",  "SPRITE"};

    for (const auto& kw : keywords) {
      CHECK(factory.getByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("REM aliases share same strategy") {
    CompilerStatementStrategyFactory factory;

    auto* rem1 = factory.getByKeyword("REM");
    auto* rem2 = factory.getByKeyword("'");

    CHECK(rem1 != nullptr);
    CHECK(rem1 == rem2);
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    CompilerStatementStrategyFactory factory;

    CHECK(factory.getByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getByKeyword("") == nullptr);
    CHECK(factory.getByKeyword("INVALID") == nullptr);
  }

  TEST_CASE("Repeated calls return same instance") {
    CompilerStatementStrategyFactory factory;

    auto* s1 = factory.getByKeyword("PRINT");
    auto* s2 = factory.getByKeyword("PRINT");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("All statement strategies have unit tests") {
    CompilerStatementStrategyFactory factory;

    CHECK(factory.size() == 69);
  }
}

TEST_SUITE("CompilerFunctionStrategyFactory") {
  TEST_CASE("All function strategies are registered") {
    CompilerFunctionStrategyFactory factory;

    std::vector<std::string> keywords = {

        "NOOPERATION",

        "TIME",        "POS",          "LPOS",   "CSRLIN",    "INKEY",
        "INKEY$",      "MAXFILES",     "FRE",    "HEAP",      "MSX",
        "NTSC",        "VDP",          "TURBO",  "COLLISION", "MAKER",
        "PLYSTATUS",

        "INT",         "FIX",          "RND",    "SIN",       "COS",
        "TAN",         "ATN",          "EXP",    "LOG",       "SQR",
        "SGN",         "ABS",          "VAL",    "PEEK",      "IPEEK",
        "VPEEK",       "INP",          "EOF",    "PSG",       "PLAY",
        "STICK",       "STRIG",        "PAD",    "PDL",       "BASE",

        "ASC",         "LEN",          "CSNG",   "CDBL",      "CINT",

        "CHR$",        "SPACE$",       "SPC",    "TAB",       "STR$",
        "INPUT$",      "BIN$",         "OCT$",   "HEX$",

        "RESOURCE",    "RESOURCESIZE",

        "USR",         "USR0",         "USR1",   "USR2",      "USR3",
        "USR4",        "USR5",         "USR6",   "USR7",      "USR8",
        "USR9",

        "POINT",       "TILE",

        "STRING$",     "LEFT$",        "RIGHT$", "MID$",      "USING$",
        "INSTR",       "VARPTR"};

    for (const auto& kw : keywords) {
      CHECK(factory.getByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    CompilerFunctionStrategyFactory factory;

    CHECK(factory.getByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getByKeyword("") == nullptr);
    CHECK(factory.getByKeyword("INVALIDFUNC") == nullptr);
  }

  TEST_CASE("SPACE$ and SPC share same handler") {
    CompilerFunctionStrategyFactory factory;

    auto* s1 = factory.getByKeyword("SPACE$");
    auto* s2 = factory.getByKeyword("SPC");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("USR variants share same handler") {
    CompilerFunctionStrategyFactory factory;

    auto* base = factory.getByKeyword("USR");

    CHECK(base != nullptr);

    CHECK(base == factory.getByKeyword("USR0"));
    CHECK(base == factory.getByKeyword("USR1"));
    CHECK(base == factory.getByKeyword("USR2"));
    CHECK(base == factory.getByKeyword("USR3"));
    CHECK(base == factory.getByKeyword("USR4"));
    CHECK(base == factory.getByKeyword("USR5"));
    CHECK(base == factory.getByKeyword("USR6"));
    CHECK(base == factory.getByKeyword("USR7"));
    CHECK(base == factory.getByKeyword("USR8"));
    CHECK(base == factory.getByKeyword("USR9"));
  }

  TEST_CASE("Repeated calls return same instance") {
    CompilerFunctionStrategyFactory factory;

    auto* s1 = factory.getByKeyword("SIN");
    auto* s2 = factory.getByKeyword("SIN");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("All function strategies have unit tests") {
    CompilerFunctionStrategyFactory factory;

    CHECK(factory.size() == 78);
  }
}

// NOLINTEND
