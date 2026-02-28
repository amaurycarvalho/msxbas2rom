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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "compiler.h"
#include "doctest/doctest.h"
#include "lexer.h"
#include "build_options.h"
#include "parser.h"
#include "pletter.h"

static int COMPILE(string filename, bool megarom = false) {
  Lexer lexer;
  Parser parser;
  Compiler compiler;
  BuildOptions opts;

  REQUIRE(lexer.lineNo == 0);

  // build options
  opts.setInputFilename(filename);
  opts.megaROM = megarom;

  // lexing
  CHECK(lexer.load(&opts) == true);
  CHECK(lexer.evaluate() == true);

  // parsing
  CHECK(parser.evaluate(&lexer) == true);

  // compiling
  CHECK(compiler.build(&parser) == true);

  return 0;
}

TEST_SUITE("Basic test") {
  TEST_CASE("PRINT statement") {
    SUBCASE("hello world") {
      string filename = "../integration/PRINT/test1.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("numerical constants") {
      string filename = "../integration/PRINT/test4.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("variables") {
      string filename = "../integration/PRINT/test7.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
  }
  TEST_CASE("MATH operators/functions") {
    SUBCASE("simple operators") {
      string filename = "../integration/MATH/test1.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("FLOAT operators") {
      string filename = "../integration/MATH/test10.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("SQR function") {
      string filename = "../integration/MATH/test11.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("RANDOMIZE functions") {
      string filename = "../integration/MATH/test13.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("BINARY operators and numeric conditionals") {
      string filename = "../integration/MATH/test14.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("INTEGER conditionals") {
      string filename = "../integration/MATH/test16.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("FLOAT conditionals") {
      string filename = "../integration/MATH/test16F.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("SIN function") {
      string filename = "../integration/MATH/test22.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("COS function") {
      string filename = "../integration/MATH/cos.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
  }
}

TEST_SUITE("GRAPH test") {
  TEST_CASE("LINE/CIRCLE/PSET/PAINT statements") {
    SUBCASE("simple plot") {
      string filename = "../integration/GRAPH/line1.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("complex plot") {
      string filename = "../integration/GRAPH/line2.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("globe") {
      string filename = "../integration/GRAPH/globe.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("mexican hat") {
      string filename = "../integration/GRAPH/mexhat.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("stars") {
      string filename = "../integration/GRAPH/stars.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("tree") {
      string filename = "../integration/GRAPH/tree.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
  }
}

TEST_SUITE("MEGAROM test") {
  TEST_CASE("PRINT statement") {
    SUBCASE("simple calculation") {
      string filename = "../integration/MEGAROM/test51.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("calculation with GOSUBs") {
      string filename = "../integration/MEGAROM/test53.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
  }
  TEST_CASE("DATA statement") {
    SUBCASE("lorem ipsum") {
      string filename = "../integration/MEGAROM/test54.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
    SUBCASE("resources") {
      string filename = "../integration/MEGAROM/data.bas";
      CAPTURE(filename);
      COMPILE(filename);
    }
  }
}

// NOLINTEND
