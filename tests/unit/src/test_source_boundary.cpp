/***
 * @file test_source_boundary.cpp
 * @brief Source document, resolver and path-policy characterization tests
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "build_options.h"
#include "build_options_resolver.h"
#include "doctest/doctest.h"
#include "file_source_resolver.h"
#include "in_memory_source_resolver.h"
#include "lexer.h"
#include "lexer_line_evaluator.h"
#include "logger.h"
#include "parser.h"
#include "source_document.h"
#include "source_resolver.h"
#include "test_temp_dir.h"

static std::string createTempFile(const TestTempDir& temp,
                                  const std::string& filename,
                                  const std::string& content) {
  const std::string path = temp.file(filename);
  std::ofstream ofs(path, std::ios::binary);
  ofs.write(content.data(), content.size());
  ofs.close();
  return path;
}

TEST_SUITE("Source boundary") {
  TEST_CASE("Lexer accepts in-memory BASIC text without a .bas file") {
    auto resolver = std::make_shared<InMemorySourceResolver>();
    resolver->add("memory.bas", "10 PRINT \"HELLO\"\n20 END\n");

    auto opts = std::make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "memory.bas");

    Lexer lexer(resolver);
    REQUIRE(lexer.load(opts) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(lexer.lines.size() == 2);
    CHECK(lexer.lines[0]->getLexemeCount() >= 3);
    CHECK(lexer.toString().find("HELLO") != std::string::npos);
  }

  TEST_CASE("In-memory and file-backed lexing are equivalent") {
    TestTempDir temp;
    const std::string content =
        "10 A=32767:B=&HFF\n20 PRINT A;B\n30 REM comment\n";
    const std::string path = createTempFile(temp, "source_boundary.bas", content);

    auto fileResolver = std::make_shared<FileSourceResolver>();
    auto memoryResolver = std::make_shared<InMemorySourceResolver>();
    memoryResolver->add(path, content);

    auto fileOpts = std::make_shared<BuildOptions>();
    build_options::setInputFilename(fileOpts, path);
    auto memoryOpts = std::make_shared<BuildOptions>();
    build_options::setInputFilename(memoryOpts, path);

    Lexer fileLexer(fileResolver);
    Lexer memoryLexer(memoryResolver);

    REQUIRE(fileLexer.load(fileOpts) == true);
    REQUIRE(memoryLexer.load(memoryOpts) == true);

    CHECK(fileLexer.lines.size() == memoryLexer.lines.size());
    CHECK(fileLexer.evaluate() == true);
    CHECK(memoryLexer.evaluate() == true);
    CHECK(fileLexer.toString() == memoryLexer.toString());
  }

  TEST_CASE("File adapter preserves missing, empty and tokenized diagnostics") {
    TestTempDir temp;
    FileSourceResolver resolver;
    SourceDocument document;

    CHECK(resolver.resolve(temp.file("source_missing.bas"), document) == false);
    CHECK(resolver.lastError() == "File doesn't exist");

    const std::string empty = createTempFile(temp, "source_empty.bas", "");
    CHECK(resolver.resolve(empty, document) == false);
    CHECK(resolver.lastError() == "Empty file");

    const std::string tokenized =
        createTempFile(temp, "source_tokenized.bas",
                       std::string("\xFF\x00\x80", 3));
    CHECK(resolver.resolve(tokenized, document) == false);
    CHECK(resolver.lastError().find("Tokenized MSX-BASIC") !=
          std::string::npos);

    const std::string invalid =
        createTempFile(temp, "source_invalid.bas",
                       std::string("\x01\x02\x03", 3));
    CHECK(resolver.resolve(invalid, document) == false);
    CHECK(resolver.lastError() == "This is not a MSX-BASIC source code file.");
  }

  TEST_CASE("INCLUDE is resolved in memory without a temporary .bas file") {
    auto resolver = std::make_shared<InMemorySourceResolver>();
    resolver->add("main.bas",
                  "INCLUDE \"child.bas\"\n10 PRINT \"MAIN\"\n20 END\n");
    resolver->add("child.bas", "5 PRINT \"CHILD\"\n");

    auto opts = std::make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "main.bas");

    auto lexer = std::make_shared<Lexer>(resolver);
    REQUIRE(lexer->load(opts) == true);
    REQUIRE(lexer->evaluate() == true);

    Parser parser;
    REQUIRE(parser.evaluate(lexer) == true);
    CHECK(parser.getTags().size() >= 3);
  }

  TEST_CASE("Nested and missing INCLUDE behavior is preserved") {
    SUBCASE("nested includes resolve through the same resolver") {
      auto resolver = std::make_shared<InMemorySourceResolver>();
      resolver->add("main.bas", "INCLUDE \"level1.bas\"\n10 END\n");
      resolver->add("level1.bas", "INCLUDE \"level2.bas\"\n");
      resolver->add("level2.bas", "5 PRINT \"DEEP\"\n");

      auto opts = std::make_shared<BuildOptions>();
      build_options::setInputFilename(opts, "main.bas");
      auto lexer = std::make_shared<Lexer>(resolver);
      REQUIRE(lexer->load(opts) == true);
      REQUIRE(lexer->evaluate() == true);

      Parser parser;
      REQUIRE(parser.evaluate(lexer) == true);
      CHECK(parser.getTags().size() == 2);
    }

    SUBCASE("missing include reports the existing diagnostic") {
      auto resolver = std::make_shared<InMemorySourceResolver>();
      resolver->add("main.bas", "INCLUDE \"missing.bas\"\n10 END\n");

      auto opts = std::make_shared<BuildOptions>();
      build_options::setInputFilename(opts, "main.bas");
      auto lexer = std::make_shared<Lexer>(resolver);
      REQUIRE(lexer->load(opts) == true);
      REQUIRE(lexer->evaluate() == true);

      Parser parser;
      CHECK(parser.evaluate(lexer) == false);
      CHECK(parser.getLogger()->trace().toString().find(
                "INCLUDE file not found or with content syntax error") !=
            std::string::npos);
    }
  }

  TEST_CASE("Build output names are derived by the application path policy") {
    BuildOptions opts;
    build_options::setInputFilename(opts, "tmp/program.bas");
    CHECK(opts.outputFilename == "tmp/program.rom");
    CHECK(opts.baseFilename == "tmp/program");

    opts.compileMode = BuildOptions::CompileMode::ASCII8;
    build_options::setInputFilename(opts, "tmp/program.bas");
    CHECK(opts.outputFilename == "tmp/program[ASCII8].rom");
    CHECK(opts.megaROM == true);

    opts.compileMode = BuildOptions::CompileMode::KonamiSCC;
    build_options::setInputFilename(opts, "tmp/program.bas");
    CHECK(opts.outputFilename == "tmp/program[KonamiSCC].rom");

    opts.compileMode = BuildOptions::CompileMode::ASCII16X;
    build_options::setInputFilename(opts, "tmp/program.bas");
    CHECK(opts.outputFilename == "tmp/program[ASCII16X].rom");
  }
}

// NOLINTEND
