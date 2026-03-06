/***
 * @file compiler_statement_context.cpp
 * @brief Compiler statement context
 */

#include "compiler_context.h"

#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_float_converter.h"
#include "compiler_statement_emitter.h"
#include "compiler_symbol_resolver.h"
#include "compiler_variable_emitter.h"

CompilerContext::CompilerContext() {
  parser = nullptr;
  opts = nullptr;
  cpu = nullptr;
  temp_str_mark = nullptr;
  heap_mark = nullptr;

  evaluator = new CompilerEvaluator(this);
  stmtEmitter = new CompilerStatementEmitter(this);
  codeHelper = new CompilerCodeHelper(this);
  fixupResolver = new CompilerFixupResolver(this);
  symbolResolver = new CompilerSymbolResolver(this);
  codeOptimizer = new CompilerCodeOptimizer(this);
  expressionEvaluator = new CompilerExpressionEvaluator(this);
  floatConverter = new CompilerFloatConverter(this);
  variableEmitter = new CompilerVariableEmitter(this);

  clear();
}

CompilerContext::~CompilerContext() {
  if (temp_str_mark) delete temp_str_mark;
  if (heap_mark) delete heap_mark;
  if (evaluator) delete evaluator;
  if (stmtEmitter) delete stmtEmitter;
  if (codeHelper) delete codeHelper;
  if (fixupResolver) delete fixupResolver;
  if (symbolResolver) delete symbolResolver;
  if (codeOptimizer) delete codeOptimizer;
  if (expressionEvaluator) delete expressionEvaluator;
  if (floatConverter) delete floatConverter;
  if (variableEmitter) delete variableEmitter;
}

void CompilerContext::clear() {
  error_message = "";
  current_tag = 0;

  mark_count = 0;
  for_count = 0;
  pt3 = false;
  akm = false;
  font = false;
  file_support = false;
  has_defusr = false;
  has_open_grp = false;
  has_tiny_sprite = false;
  compiled = false;

  end_mark = nullptr;
  enable_basic_mark = nullptr;
  disable_basic_mark = nullptr;
  draw_mark = nullptr;
  io_redirect_mark = nullptr;
  io_screen_mark = nullptr;

  symbols.clear();
  fixes.clear();

  symbolManager.clear();
  resourceManager.clear();

  while (!forNextStack.empty()) forNextStack.pop();

  if (!temp_str_mark) {
    temp_str_mark = new SymbolNode();
    temp_str_mark->lexeme =
        new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric,
                   "_TEMPSTR_START_", "0");
    temp_str_mark->lexeme->isAbstract = true;
  }

  if (!heap_mark) {
    heap_mark = new SymbolNode();
    heap_mark->lexeme = new Lexeme(Lexeme::type_identifier,
                                   Lexeme::subtype_numeric, "_HEAP_", "0");
    heap_mark->lexeme->isAbstract = true;
  }
}

void CompilerContext::syntaxError() {
  syntaxError("Syntax error");
}

void CompilerContext::syntaxError(string msg) {
  compiled = false;
  error_message = msg;
  if (current_tag) error_message += " (line=" + current_tag->value + ")";
}
