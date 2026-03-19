/***
 * @file test_symbols.cpp
 * @brief MSXBAS2ROM symbols unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "build_options.h"
#include "code_node.h"
#include "doctest/doctest.h"
#include "fswrapper.h"
#include "lexeme.h"
#include "symbol_export_context.h"
#include "symbol_export_strategy_factory.h"
#include "symbol_manager.h"

static void ensureTmpDir() {
#ifdef _WIN32
  _mkdir("tmp");
#else
  mkdir("tmp", 0755);
#endif
}

static std::string readFileText(const std::string& filename) {
  std::ifstream ifs(filename.c_str());
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return content;
}

struct SymbolsFixture {
  SymbolManager manager;
  std::vector<shared_ptr<CodeNode>> nodes;
  std::vector<shared_ptr<Lexeme>> lexemes;

  SymbolsFixture() {
    shared_ptr<CodeNode> code = make_shared<CodeNode>();
    code->name = "TESTCODE";
    code->start = 0;
    code->length = 2;
    code->segm = 1;
    code->addr_within_segm = 0x1234;
    code->is_code = true;
    code->debug = true;
    code->lexeme = NULL;
    manager.context->codeList.push_back(code);
    nodes.push_back(code);

    shared_ptr<Lexeme> lex = make_shared<Lexeme>();
    lex->subtype = Lexeme::subtype_numeric;
    lex->isArray = false;
    lex->x_size = 0;
    lex->y_size = 0;
    lexemes.push_back(lex);

    shared_ptr<CodeNode> data = make_shared<CodeNode>();
    data->name = "TESTVAR";
    data->start = 0;
    data->length = 4;
    data->segm = 0;
    data->addr_within_segm = 0x4000;
    data->is_code = false;
    data->debug = true;
    data->lexeme = lex;
    manager.context->dataList.push_back(data);
    nodes.push_back(data);
  }

  ~SymbolsFixture() = default;
};

TEST_SUITE("Symbols") {
  TEST_CASE("SymbolManager clears lists") {
    SymbolManager manager;
    shared_ptr<CodeNode> code = make_shared<CodeNode>();
    shared_ptr<CodeNode> data = make_shared<CodeNode>();
    manager.context->codeList.push_back(code);
    manager.context->dataList.push_back(data);

    manager.context->clear();
    CHECK(manager.context->codeList.empty() == true);
    CHECK(manager.context->dataList.empty() == true);
  }

  TEST_CASE("Kernel symbols include LOADER") {
    SymbolManager manager;
    std::vector<std::vector<std::string>> kernel =
        manager.context->kernelSymbolAddresses;
    CHECK(kernel.size() > 0);

    bool found = false;
    for (size_t i = 0; i < kernel.size(); i++) {
      if (kernel[i].size() > 0 && kernel[i][0] == "LOADER") {
        found = true;
        break;
      }
    }
    CHECK(found == true);
  }

  TEST_CASE("Symbol export strategy factory returns expected types") {
    SymbolExportStrategyFactory factory;
    CHECK(factory.size() == 5);
    CHECK(factory.getBySymbolMode(BuildOptions::SymbolsMode::Symbol) != NULL);
    CHECK(factory.getBySymbolMode(BuildOptions::SymbolsMode::Omds) != NULL);
    CHECK(factory.getBySymbolMode(BuildOptions::SymbolsMode::NoICE) != NULL);
    CHECK(factory.getBySymbolMode(BuildOptions::SymbolsMode::Cdb) != NULL);
    CHECK(factory.getBySymbolMode(BuildOptions::SymbolsMode::Elf) != NULL);
    CHECK(factory.getBySymbolMode(BuildOptions::SymbolsMode::None) == NULL);
  }

  TEST_CASE("Symbol manager saves .symbol file") {
    ensureTmpDir();
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->setInputFilename("tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Symbol;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fileExists(fixture.manager.context->exportFilename) == true);

    std::string content = readFileText(fixture.manager.context->exportFilename);
    CHECK(content.find("LOADER") != std::string::npos);
    CHECK(content.find("TESTCODE") != std::string::npos);
    CHECK(content.find("TESTVAR") != std::string::npos);

    std::remove(fixture.manager.context->exportFilename.c_str());
  }

  TEST_CASE("Symbol manager saves .omds file") {
    ensureTmpDir();
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->setInputFilename("tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Omds;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fileExists(fixture.manager.context->exportFilename) == true);

    std::string content = readFileText(fixture.manager.context->exportFilename);
    CHECK(content.find("<DebugSession") != std::string::npos);
    CHECK(content.find("<Symbols>") != std::string::npos);
    CHECK(content.find("TESTCODE") != std::string::npos);

    std::remove(fixture.manager.context->exportFilename.c_str());
  }

  TEST_CASE("Symbol manager saves .noi file") {
    ensureTmpDir();
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->setInputFilename("tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::NoICE;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fileExists(fixture.manager.context->exportFilename) == true);

    std::string content = readFileText(fixture.manager.context->exportFilename);
    CHECK(content.find("def LOADER") != std::string::npos);
    CHECK(content.find("def TESTCODE") != std::string::npos);
    CHECK(content.find("def TESTVAR") != std::string::npos);

    std::remove(fixture.manager.context->exportFilename.c_str());
  }

  TEST_CASE("Symbol manager saves .cdb file") {
    ensureTmpDir();
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->setInputFilename("tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Cdb;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fileExists(fixture.manager.context->exportFilename) == true);

    std::string content = readFileText(fixture.manager.context->exportFilename);
    CHECK(content.find("S:G$TESTVAR") != std::string::npos);
    CHECK(content.find("L:G$TESTVAR") != std::string::npos);

    std::remove(fixture.manager.context->exportFilename.c_str());
  }

  TEST_CASE("Symbol manager saves .elf file") {
    ensureTmpDir();

    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();

    opts->setInputFilename("tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Elf;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);

    CHECK(fileExists(fixture.manager.context->exportFilename) == true);

    std::ifstream ifs(fixture.manager.context->exportFilename.c_str(),
                      std::ios::binary);

    REQUIRE(ifs.good());

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(ifs)),
                                    std::istreambuf_iterator<char>());

    REQUIRE(data.size() > 64);

    /* ---------------------------------------------------------
     * Validate ELF header
     * --------------------------------------------------------- */

    CHECK(data[0] == 0x7f);
    CHECK(data[1] == 'E');
    CHECK(data[2] == 'L');
    CHECK(data[3] == 'F');

    /* 32-bit ELF */

    CHECK(data[4] == 1);

    /* little endian */

    CHECK(data[5] == 1);

    /* version */

    CHECK(data[6] == 1);

    /* ---------------------------------------------------------
     * Validate symbol names exist somewhere in file
     * --------------------------------------------------------- */

    std::string fileContent(data.begin(), data.end());

    CHECK(fileContent.find("TESTCODE") != std::string::npos);
    CHECK(fileContent.find("TESTVAR") != std::string::npos);

    /* ---------------------------------------------------------
     * cleanup
     * --------------------------------------------------------- */

    std::remove(fixture.manager.context->exportFilename.c_str());
  }

  TEST_CASE("Symbol manager returns false when symbols mode is none") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->setInputFilename("tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::None;

    CHECK(fixture.manager.saveSymbol(opts) == false);
  }
}

// NOLINTEND
