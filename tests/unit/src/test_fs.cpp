/***
 * @file test.cpp
 * @brief MSXBAS2ROM unit testing
 * @author Amaury Carvalho (2019-2025)
 * @note
 * doctest reference:
 *   https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
 * compiling:
 *   make all
 * testing:
 *   ./test
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include "build_options.h"
#include "build_options_resolver.h"
#include "compiler.h"
#include "doctest/doctest.h"
#include "in_memory_source_resolver.h"
#include "test_support.h"
#include "test_temp_dir.h"
#include <fstream>
#include <iterator>
#include "fswrapper.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "pletter.h"
#include "z80.h"

TEST_SUITE("Fswrapper") {
  TEST_CASE("fileExists detects existing files") {
    TestTempDir temp;
    const std::string existing = temp.file("exists.bas");
    std::ofstream out(existing.c_str());
    out << "10 END\n";
    out.close();
    CHECK(fileExists(existing) == true);
    CHECK(fileExists(temp.file("nonexistent_file_xyz.bas")) == false);
  }

  TEST_CASE("pathJoin handles empty and separator edge cases") {
    std::string sep(1, fsFolderSeparator);
    CHECK(pathJoin("", "file.txt") == "file.txt");
    CHECK(pathJoin("base", "") == "base");
    CHECK(pathJoin("", "") == "");

    std::string joined = pathJoin("dir", "file.txt");
    CHECK(joined == "dir" + sep + "file.txt");

    std::string joinedTrail = pathJoin("dir/", "file.txt");
    CHECK(joinedTrail == "dir" + sep + "file.txt");

    std::string joinedLead = pathJoin("dir", sep + std::string("file.txt"));
    CHECK(joinedLead == "dir" + sep + "file.txt");
  }

  TEST_CASE("path filename helpers split correctly") {
    CHECK(getFilePath("/a/b/c.bas") == "/a/b");
    CHECK(getFileName("/a/b/c.bas") == "c.bas");
    CHECK(getFileExtension("/a/b/c.bas") == ".bas");
    CHECK(getFileNameWithoutExtension("/a/b/c.bas") == "c");
    CHECK(getFileExtension("c") == "");
    CHECK(getFileNameWithoutExtension("c") == "c");
  }

  TEST_CASE("removeQuotes strips surrounding quotes") {
    CHECK(removeQuotes("\"hello\"") == "hello");
    CHECK(removeQuotes("\"hello") == "hello");
    CHECK(removeQuotes("hello\"") == "hello");
    CHECK(removeQuotes("hello") == "hello");
    CHECK(removeQuotes("") == "");
  }

  TEST_CASE("pathExists detects directories only") {
    TestTempDir temp;
    const std::string file = temp.file("not_a_dir.txt");
    std::ofstream out(file.c_str());
    out << "x";
    out.close();
    CHECK(pathExists(temp.path()) == true);
    CHECK(pathExists(temp.file("nonexistent_dir_xyz")) == false);
    CHECK(pathExists(file) == false);
  }

  TEST_CASE("Logger contain filters by level") {
    Logger logger;
    logger.error("boom");
    logger.warning("warn");

    CHECK(logger.contain(set<Logger::LogLevel>{Logger::LogLevel::ERR}) == true);
    CHECK(logger.contain(set<Logger::LogLevel>{Logger::LogLevel::WARNING}) ==
          true);
    CHECK(logger.contain(set<Logger::LogLevel>{Logger::LogLevel::DEBUG}) ==
          false);
  }
}

// NOLINTEND
