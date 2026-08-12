/***
 * @file test_compiler_expressions.cpp
 * @brief MSXBAS2ROM compiler expression evaluator unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "build_options.h"
#include "compiler.h"
#include "compiler_context.h"
#include "compiler_statement_strategy_factory.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
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

TEST_SUITE("CompilerExpressionEvaluator") {
  TEST_CASE("Integer literal formats compile") {
    SUBCASE("Hex literal") {
      std::string errors;
      bool ok = compileStatementProgram("expr_hex.bas", "10 A=&HFF\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Octal literal") {
      std::string errors;
      bool ok = compileStatementProgram("expr_oct.bas", "10 A=&O17\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Binary literal") {
      std::string errors;
      bool ok = compileStatementProgram("expr_bin.bas", "10 A=&B1010\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Arithmetic operators compile") {
    SUBCASE("MOD with integers") {
      std::string errors;
      bool ok = compileStatementProgram("expr_mod.bas", "10 A=10 MOD 3\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("MOD with float left") {
      std::string errors;
      bool ok = compileStatementProgram("expr_mod_float.bas",
                                        "10 A=10.5 MOD 3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SHR") {
      std::string errors;
      bool ok = compileStatementProgram("expr_shr.bas", "10 A=8 SHR 2\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SHL") {
      std::string errors;
      bool ok = compileStatementProgram("expr_shl.bas", "10 A=1 SHL 4\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("EQV") {
      std::string errors;
      bool ok = compileStatementProgram("expr_eqv.bas", "10 A=1 EQV 2\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IMP") {
      std::string errors;
      bool ok = compileStatementProgram("expr_imp.bas", "10 A=1 IMP 2\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Integer division") {
      std::string errors;
      bool ok = compileStatementProgram("expr_idiv.bas", "10 A=10\\3\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Integer division with float left") {
      std::string errors;
      bool ok = compileStatementProgram("expr_idiv_float.bas",
                                        "10 A=10.5\\3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Power with integer base") {
      std::string errors;
      bool ok = compileStatementProgram("expr_pow_int.bas", "10 A=2^3\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Power with float base") {
      std::string errors;
      bool ok = compileStatementProgram("expr_pow_float.bas",
                                        "10 A=2.5^2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Power with float exponent") {
      std::string errors;
      bool ok = compileStatementProgram("expr_pow_fexp.bas",
                                        "10 A=2^3.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Unary operators compile") {
    SUBCASE("NOT") {
      std::string errors;
      bool ok = compileStatementProgram("expr_not.bas", "10 A=NOT 1\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Unary minus integer") {
      std::string errors;
      bool ok = compileStatementProgram("expr_neg_int.bas", "10 A=-5\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Unary minus float") {
      std::string errors;
      bool ok = compileStatementProgram("expr_neg_float.bas",
                                        "10 A=-5.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Unary plus") {
      std::string errors;
      bool ok = compileStatementProgram("expr_unary_plus.bas",
                                        "10 A=+5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Comparison operators compile") {
    SUBCASE("Integer comparisons") {
      const char* ops[] = {"=", "<>", "<", "<=", ">", ">="};
      for (const char* op : ops) {
        std::string program = "10 IF 1 " + std::string(op) +
                              " 2 THEN PRINT 1\n20 END\n";
        std::string errors;
        bool ok = compileStatementProgram(
            "expr_cmp_int.bas", program, &errors);
        CHECK(ok == true);
        if (!ok) CHECK(errors.find(std::string(op)) != std::string::npos);
      }
    }

    SUBCASE("Float comparisons") {
      const char* ops[] = {"=", "<>", "<", "<=", ">", ">="};
      for (const char* op : ops) {
        std::string program = "10 IF 1.5 " + std::string(op) +
                              " 2.5 THEN PRINT 1\n20 END\n";
        std::string errors;
        bool ok = compileStatementProgram(
            "expr_cmp_float.bas", program, &errors);
        CHECK(ok == true);
        if (!ok) CHECK(errors.find(std::string(op)) != std::string::npos);
      }
    }

    SUBCASE("String comparisons") {
      const char* ops[] = {"=", "<>", "<", "<=", ">", ">="};
      for (const char* op : ops) {
        std::string program = "10 IF \"A\" " + std::string(op) +
                              " \"B\" THEN PRINT 1\n20 END\n";
        std::string errors;
        bool ok = compileStatementProgram(
            "expr_cmp_str.bas", program, &errors);
        CHECK(ok == true);
        if (!ok) CHECK(errors.find(std::string(op)) != std::string::npos);
      }
    }
  }

  TEST_CASE("Float arithmetic operators compile") {
    SUBCASE("Float addition") {
      std::string errors;
      bool ok = compileStatementProgram("expr_fadd.bas",
                                        "10 A=1.5+2.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float subtraction") {
      std::string errors;
      bool ok = compileStatementProgram("expr_fsub.bas",
                                        "10 A=2.5-1.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float multiplication") {
      std::string errors;
      bool ok = compileStatementProgram("expr_fmul.bas",
                                        "10 A=1.5*2.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float division") {
      std::string errors;
      bool ok = compileStatementProgram("expr_fdiv.bas",
                                        "10 A=2.5/2.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("String concatenation compiles") {
    std::string errors;
    bool ok = compileStatementProgram("expr_concat.bas",
                                      "10 A$=\"A\"+\"B\"\n20 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("Multiplication optimization paths compile") {
    SUBCASE("Multiply by 0") {
      std::string errors;
      bool ok = compileStatementProgram("mul_0.bas", "10 A=5*0\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Multiply by 1") {
      std::string errors;
      bool ok = compileStatementProgram("mul_1.bas", "10 A=5*1\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Multiply by 2") {
      std::string errors;
      bool ok = compileStatementProgram("mul_2.bas", "10 A=5*2\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Multiply by 128") {
      std::string errors;
      bool ok = compileStatementProgram("mul_128.bas", "10 A=5*128\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Multiply by 256") {
      std::string errors;
      bool ok = compileStatementProgram("mul_256.bas", "10 A=5*256\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Multiply by generic constant") {
      std::string errors;
      bool ok = compileStatementProgram("mul_300.bas", "10 A=5*300\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Division optimization paths compile") {
    const int divisors[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 300};
    for (int d : divisors) {
      std::string program =
          "10 A=5/" + std::to_string(d) + "\n20 END\n";
      std::string errors;
      bool ok = compileStatementProgram("div_path.bas", program, &errors);
      CHECK(ok == true);
      if (!ok) CHECK(errors.find("divide") != std::string::npos);
    }
  }

  TEST_CASE("Type casting paths compile") {
    SUBCASE("Integer plus float") {
      std::string errors;
      bool ok = compileStatementProgram("cast_int_float.bas",
                                        "10 A=1+2.5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float plus integer") {
      std::string errors;
      bool ok = compileStatementProgram("cast_float_int.bas",
                                        "10 A=2.5+1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Integer plus string") {
      std::string errors;
      bool ok = compileStatementProgram("cast_int_str.bas",
                                        "10 A=1+\"2\"\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("String plus integer") {
      std::string errors;
      bool ok = compileStatementProgram("cast_str_int.bas",
                                        "10 A=\"2\"+1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float plus string is rejected") {
      std::string errors;
      bool ok = compileStatementProgram("cast_float_str.bas",
                                        "10 A=1.5+\"2\"\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("Unknown type to cast") != std::string::npos);
    }

    SUBCASE("Integer compared to string") {
      std::string errors;
      bool ok = compileStatementProgram(
          "cast_cmp_int_str.bas", "10 IF 1=\"1\" THEN PRINT 1\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Function calls within expressions compile") {
    std::string errors;
    bool ok = compileStatementProgram(
        "expr_func_call.bas", "10 A=SIN(1)+COS(2)\n20 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("Nested parentheses compile") {
    std::string errors;
    bool ok = compileStatementProgram(
        "expr_nested_paren.bas", "10 A=((1+2)*3)\n20 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("Array element read in expression") {
    std::string errors;
    bool ok = compileStatementProgram(
        "expr_array_read.bas",
        "10 DIM A(5)\n20 A(0)=1\n30 B=A(0)\n40 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("USR function call compiles") {
    std::string errors;
    bool ok = compileStatementProgram(
        "expr_usr.bas", "10 DEF USR=1\n20 A=USR(1)\n30 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("Unknown function in expression is rejected") {
    std::string errors;
    bool ok = compileStatementProgram(
        "expr_unknown_func.bas", "10 A=FOO(1)\n20 END\n", &errors);
    CHECK(ok == false);
    CHECK(errors.find("Undeclared array or unknown function") !=
          std::string::npos);
  }

  TEST_CASE("Invalid type combinations are rejected") {
    SUBCASE("NOT on string") {
      std::string errors;
      bool ok = compileStatementProgram("expr_not_str.bas",
                                        "10 A=NOT \"X\"\n20 END\n", &errors);
      CHECK(ok == false);
    }

    SUBCASE("Unary minus on string") {
      std::string errors;
      bool ok = compileStatementProgram("expr_neg_str.bas",
                                        "10 A=-\"X\"\n20 END\n", &errors);
      CHECK(ok == false);
    }
  }

  TEST_CASE("Arithmetic optimizations in MegaROM mode") {
    SUBCASE("Multiply by constant in MegaROM") {
      const std::string path =
          createTempBas("opt_mul_mega.bas", "10 A=5*128\n20 END\n");

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

    SUBCASE("Multiply by 256 in MegaROM") {
      const std::string path =
          createTempBas("opt_mul256_mega.bas", "10 A=5*256\n20 END\n");

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

    SUBCASE("Divide by constant in MegaROM") {
      const std::string path =
          createTempBas("opt_div_mega.bas", "10 A=5/8\n20 END\n");

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
}

TEST_SUITE("CompilerVariableEmitter") {
  TEST_CASE("Keyword assignments compile") {
    SUBCASE("TIME assignment") {
      std::string errors;
      bool ok = compileStatementProgram("var_time.bas", "10 TIME=1\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("ERR assignment") {
      std::string errors;
      bool ok = compileStatementProgram("var_err.bas", "10 ERR=1\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("MAXFILES assignment") {
      std::string errors;
      bool ok = compileStatementProgram("var_maxfiles.bas",
                                        "10 MAXFILES=5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Two-dimensional array indexing compiles") {
    SUBCASE("Literal indices") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_lit.bas",
          "10 DIM A(3,3)\n20 A(1,2)=5\n30 B=A(1,2)\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Variable indices") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_var.bas",
          "10 DIM A(3,3)\n20 X=1\n30 A(X,X)=5\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float element assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_float.bas",
          "10 DIM A#(3,3)\n20 A#(1,2)=5.5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Array element assignments of different types") {
    SUBCASE("Numeric array float assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_float.bas", "10 DIM A(3)\n20 A(1)=2.5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Numeric array read into float") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_read_float.bas",
          "10 DIM A(3)\n20 A(0)=1.5\n30 B=A(0)\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Numeric array element read") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_read.bas",
          "10 DIM A(5)\n20 A(2)=42\n30 B=A(2)\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Array index expression compiles") {
    std::string errors;
    bool ok = compileStatementProgram(
        "var_arr_index_expr.bas",
        "10 DIM A(5)\n20 X=2\n30 A(X+1)=42\n40 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("Two-dimensional array factor variants compile") {
    SUBCASE("2x3 dimension") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_23.bas", "10 DIM A(2,3)\n20 A(1,2)=5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("10x5 dimension") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_105.bas", "10 DIM A(10,5)\n20 A(1,2)=5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("3x10 dimension") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_310.bas", "10 DIM A(3,10)\n20 A(2,2)=5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("255x3 dimension") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_2553.bas", "10 DIM A(255,3)\n20 A(1,2)=5\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("2D with variable indices") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_varidx.bas",
          "10 DIM A(2,2)\n20 X=1:Y=2\n30 A(X,Y)=5\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Float array element assignment variants compile") {
    SUBCASE("Float 2D array literal index") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_2d_float_idx.bas",
          "10 DIM A#(3,3)\n20 A#(1,2)=5.5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float array read into variable") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_float_read.bas",
          "10 DIM A#(3)\n20 A#(0)=1.5\n30 B=A#(0)\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("Array index missing is rejected") {
    std::string errors;
    bool ok = compileStatementProgram(
        "var_arr_no_index.bas", "10 DIM A(5)\n20 A()=42\n30 END\n", &errors);
    CHECK(ok == false);
    CHECK(errors.find("Array index is missing") != std::string::npos);
  }

  TEST_CASE("Array index type variants") {
    SUBCASE("Float literal index") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_float_idx.bas", "10 DIM A(5)\n20 A(1.5)=5\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float literal 2D index") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_2d_float_idx.bas", "10 DIM A(3,3)\n20 A(1.5,2.5)=5\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("String index is rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_str_idx.bas", "10 DIM A(5)\n20 A(\"X\")=5\n30 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.find("Invalid array index type") != std::string::npos);
    }

    SUBCASE("String 2D index is rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "var_arr_2d_str_idx.bas", "10 DIM A(3,3)\n20 A(1,\"X\")=5\n30 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.find("Invalid array 2nd index type") != std::string::npos);
    }
  }
}

TEST_SUITE("CompilerFloatConverter") {
  TEST_CASE("USING$ format strings compile") {
    struct UsingCase {
      const char* name;
      const char* format;
    };

    const UsingCase cases[] = {
        {"HASH", "###"},
        {"ZEROS", "000"},
        {"PLUS", "+###"},
        {"MINUS", "-###"},
        {"COMMA", "#,###"},
        {"DOLLAR", "$###"},
        {"STAR", "**#"},
        {"CARET", "###^"},
        {"DECIMAL", "###.##"},
        {"MIXED", "+#,###.##"},
    };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        std::string program = "10 PRINT USING$(\"" +
                              std::string(test_case.format) +
                              "\", 12.5)\n20 END\n";
        std::string errors;
        bool ok = compileStatementProgram(
            "using_format.bas", program, &errors);
        CHECK(ok == true);
        CHECK(errors.empty());
      }
    }
  }
}

// NOLINTEND
