/***
 * @file test_symbols.cpp
 * @brief MSXBAS2ROM symbols unit testing
 */

// NOLINTBEGIN
// test-inventory: file-adapter

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
#include "build_options_resolver.h"
#include "code_node.h"
#include "doctest/doctest.h"
#include "file_symbol_output_sink.h"
#include "fswrapper.h"
#include "in_memory_symbol_output_sink.h"
#include "lexeme.h"
#include "symbol_export_context.h"
#include "symbol_export_strategy_factory.h"
#include "symbol_manager.h"
#include "test_temp_dir.h"

struct SymbolsFixture {
  std::shared_ptr<InMemorySymbolOutputSink> sink;
  SymbolManager manager;
  std::vector<shared_ptr<CodeNode>> nodes;
  std::vector<shared_ptr<Lexeme>> lexemes;

  SymbolsFixture()
      : sink(std::make_shared<InMemorySymbolOutputSink>()), manager(sink) {
    shared_ptr<CodeNode> code = make_shared<CodeNode>();
    code->name = "TESTCODE";
    code->start = 0;
    code->length = 2;
    code->segm = 1;
    code->addr_within_segm = 0x1234;
    code->is_code = true;
    code->debug = true;
    code->lexeme = NULL;
    manager.context->getCodeList().push_back(code);
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
    manager.context->getDataList().push_back(data);
    nodes.push_back(data);
  }

  ~SymbolsFixture() = default;

  std::string text() const {
    return sink->text(manager.context->getExportFilename());
  }

  std::vector<unsigned char> binary() const {
    return sink->binary(manager.context->getExportFilename());
  }
};

TEST_SUITE("Symbols") {
  TEST_CASE("SymbolManager clears lists") {
    auto sink = make_shared<InMemorySymbolOutputSink>();
    SymbolManager manager(sink);
    shared_ptr<CodeNode> code = make_shared<CodeNode>();
    shared_ptr<CodeNode> data = make_shared<CodeNode>();
    manager.context->getCodeList().push_back(code);
    manager.context->getDataList().push_back(data);

    manager.context->clear();
    CHECK(manager.context->getCodeList().empty() == true);
    CHECK(manager.context->getDataList().empty() == true);
  }

  TEST_CASE("Kernel symbols include LOADER") {
    auto sink = make_shared<InMemorySymbolOutputSink>();
    SymbolManager manager(sink);
    std::vector<std::vector<std::string>> kernel =
        manager.context->getKernelSymbolAddresses();
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

  TEST_CASE("Symbol exports include array data descriptors") {
    SymbolsFixture fixture;
    shared_ptr<Lexeme> arrLex = make_shared<Lexeme>();
    arrLex->subtype = Lexeme::subtype_numeric;
    arrLex->isArray = true;
    arrLex->x_size = 3;
    arrLex->y_size = 2;

    shared_ptr<CodeNode> arrData = make_shared<CodeNode>();
    arrData->name = "ARRVAR";
    arrData->start = 0;
    arrData->length = 12;
    arrData->segm = 0;
    arrData->addr_within_segm = 0x5000;
    arrData->is_code = false;
    arrData->debug = true;
    arrData->lexeme = arrLex;
    fixture.manager.context->getDataList().push_back(arrData);

    BuildOptions::SymbolsMode modes[] = {
        BuildOptions::SymbolsMode::Symbol, BuildOptions::SymbolsMode::Omds,
        BuildOptions::SymbolsMode::NoICE,  BuildOptions::SymbolsMode::Cdb,
        BuildOptions::SymbolsMode::Elf};
    for (BuildOptions::SymbolsMode mode : modes) {
      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      build_options::setInputFilename(opts, "tmp/symbols_test.bas");
      opts->symbols = mode;

      REQUIRE(fixture.manager.saveSymbol(opts) == true);
      if (mode == BuildOptions::SymbolsMode::Elf) {
        CHECK(fixture.sink->hasBinary(
                  fixture.manager.context->getExportFilename()) == true);
      } else {
        CHECK(fixture.text().find("ARRVAR") != std::string::npos);
      }
    }
  }

  TEST_CASE("Symbol exports with data node without lexeme") {
    SymbolsFixture fixture;
    shared_ptr<CodeNode> rawData = make_shared<CodeNode>();
    rawData->name = "RAWVAR";
    rawData->start = 0;
    rawData->length = 4;
    rawData->segm = 0;
    rawData->addr_within_segm = 0x6000;
    rawData->is_code = false;
    rawData->debug = true;
    rawData->lexeme = NULL;
    fixture.manager.context->getDataList().push_back(rawData);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Cdb;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fixture.text().find("RAWVAR") != std::string::npos);
  }

  TEST_CASE("Symbol manager saves .symbol file") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Symbol;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fixture.sink->hasText(
              fixture.manager.context->getExportFilename()) == true);

    std::string content = fixture.text();
    CHECK(content.find("LOADER") != std::string::npos);
    CHECK(content.find("TESTCODE") != std::string::npos);
    CHECK(content.find("TESTVAR") != std::string::npos);
  }

  TEST_CASE("Symbol manager saves .omds file") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Omds;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    std::string content = fixture.text();
    CHECK(content.find("<DebugSession") != std::string::npos);
    CHECK(content.find("<Symbols>") != std::string::npos);
    CHECK(content.find("TESTCODE") != std::string::npos);
  }

  TEST_CASE("Symbol manager saves .noi file") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::NoICE;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    std::string content = fixture.text();
    CHECK(content.find("def LOADER") != std::string::npos);
    CHECK(content.find("def TESTCODE") != std::string::npos);
    CHECK(content.find("def TESTVAR") != std::string::npos);
  }

  TEST_CASE("Symbol manager saves .cdb file") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Cdb;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    std::string content = fixture.text();
    CHECK(content.find("S:G$TESTVAR") != std::string::npos);
    CHECK(content.find("L:G$TESTVAR") != std::string::npos);
  }

  TEST_CASE("Symbol manager saves .elf file") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();

    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::Elf;

    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fixture.sink->hasBinary(
              fixture.manager.context->getExportFilename()) == true);

    std::vector<unsigned char> data = fixture.binary();
    REQUIRE(data.size() > 64);

    CHECK(data[0] == 0x7f);
    CHECK(data[1] == 'E');
    CHECK(data[2] == 'L');
    CHECK(data[3] == 'F');
    CHECK(data[4] == 1);
    CHECK(data[5] == 1);
    CHECK(data[6] == 1);

    std::string fileContent(data.begin(), data.end());
    CHECK(fileContent.find("TESTCODE") != std::string::npos);
    CHECK(fileContent.find("TESTVAR") != std::string::npos);
  }

  TEST_CASE("Symbol manager writes through an in-memory sink") {
    SymbolsFixture fixture;

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbol_sink.bas");

    BuildOptions::SymbolsMode textModes[] = {
        BuildOptions::SymbolsMode::Symbol, BuildOptions::SymbolsMode::Omds,
        BuildOptions::SymbolsMode::NoICE, BuildOptions::SymbolsMode::Cdb};
    for (BuildOptions::SymbolsMode mode : textModes) {
      opts->symbols = mode;
      REQUIRE(fixture.manager.saveSymbol(opts) == true);
      CHECK(fixture.sink->hasText(
                fixture.manager.context->getExportFilename()) == true);
      CHECK(fileExists(fixture.manager.context->getExportFilename()) == false);
    }

    opts->symbols = BuildOptions::SymbolsMode::Elf;
    REQUIRE(fixture.manager.saveSymbol(opts) == true);
    CHECK(fixture.sink->hasBinary(
              fixture.manager.context->getExportFilename()) == true);
    CHECK(fixture.binary().size() > 64);
    CHECK(fileExists(fixture.manager.context->getExportFilename()) == false);
  }

  TEST_CASE("File sink writes and fails explicitly") {
    TestTempDir temp;
    SymbolsFixture fixture;
    auto fileSink = std::make_shared<FileSymbolOutputSink>();
    SymbolManager fileManager(fileSink);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, temp.file("file_sink_test.bas"));
    opts->symbols = BuildOptions::SymbolsMode::Symbol;
    REQUIRE(fileManager.saveSymbol(opts) == true);
    CHECK(fileExists(fileManager.context->getExportFilename()) == true);

    CHECK(fileSink->writeText(temp.file("no_such_dir/file.sym"), "x") ==
          false);
  }

  TEST_CASE("Symbol manager returns false when symbols mode is none") {
    SymbolsFixture fixture;
    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/symbols_test.bas");
    opts->symbols = BuildOptions::SymbolsMode::None;

    CHECK(fixture.manager.saveSymbol(opts) == false);
  }
}

// NOLINTEND
