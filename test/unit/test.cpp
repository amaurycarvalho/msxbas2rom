//---------------------------------------------------------------------------
// MSXBAS2ROM unit testing
//---------------------------------------------------------------------------
// By Amaury Carvalho (2025)
// doctest reference:
//   https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
// compiling:
//   make all
// testing:
//   ./test
//---------------------------------------------------------------------------

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "lex.h"
#include "parse.h"
#include "compiler.h"
#include "header.h"
#include "pletter.h"

static int COMPILE(char *filename, bool megarom = false ) {
    Lexer lexer;
    Parser parser;
    Compiler compiler;

    REQUIRE(lexer.lineNo == 0);

    // lexing
    CHECK(lexer.load(filename) == true);
    CHECK(lexer.evaluate() == true);

    // parsing
    CHECK(parser.evaluate(&lexer) == true);

    // compiling
    compiler.megaROM = megarom;
    CHECK(compiler.build(&parser) == true);

    return 0;
}

TEST_SUITE("Basic test") {
    TEST_CASE("PRINT statement") {
        SUBCASE("hello world") {
            char filename[] = "../integration/PRINT/test1.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("numerical constants") {
            char filename[] = "../integration/PRINT/test4.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("variables") {
            char filename[] = "../integration/PRINT/test7.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
    }
    TEST_CASE("MATH operators/functions") {
        SUBCASE("simple operators") {
            char filename[] = "../integration/MATH/test1.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("FLOAT operators") {
            char filename[] = "../integration/MATH/test10.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("SQR function") {
            char filename[] = "../integration/MATH/test11.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("RANDOMIZE functions") {
            char filename[] = "../integration/MATH/test13.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("BINARY operators and numeric conditionals") {
            char filename[] = "../integration/MATH/test14.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("INTEGER conditionals") {
            char filename[] = "../integration/MATH/test16.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("FLOAT conditionals") {
            char filename[] = "../integration/MATH/test16F.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("SIN function") {
            char filename[] = "../integration/MATH/test22.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("COS function") {
            char filename[] = "../integration/MATH/cos.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
    }
}

TEST_SUITE("GRAPH test") {
    TEST_CASE("LINE/CIRCLE/PSET/PAINT statements") {
        SUBCASE("simple plot") {
            char filename[] = "../integration/GRAPH/line1.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("complex plot") {
            char filename[] = "../integration/GRAPH/line2.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("globe") {
            char filename[] = "../integration/GRAPH/globe.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("mexican hat") {
            char filename[] = "../integration/GRAPH/mexhat.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("stars") {
            char filename[] = "../integration/GRAPH/stars.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("tree") {
            char filename[] = "../integration/GRAPH/tree.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
    }
}

TEST_SUITE("MEGAROM test") {
    TEST_CASE("PRINT statement") {
        SUBCASE("simple calculation") {
            char filename[] = "../integration/MEGAROM/test51.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("calculation with GOSUBs") {
            char filename[] = "../integration/MEGAROM/test53.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
    }
    TEST_CASE("DATA statement") {
        SUBCASE("lorem ipsum") {
            char filename[] = "../integration/MEGAROM/test54.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
        SUBCASE("resources") {
            char filename[] = "../integration/MEGAROM/data.bas";
            CAPTURE(filename);
            COMPILE(filename);
        }
    }
}



