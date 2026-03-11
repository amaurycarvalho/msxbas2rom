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
#include "compiler_symbol_resolver.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"
#include "logger.h"
#include "parser.h"
#include "resources.h"
#include "symbol_manager.h"

CompilerContext::CompilerContext() {
  parser = nullptr;
  opts = nullptr;
  cpu = nullptr;
  temp_str_mark = nullptr;
  heap_mark = nullptr;

  logger.reset(new Logger());
  evaluator.reset(new CompilerEvaluator(this));
  codeHelper.reset(new CompilerCodeHelper(this));
  fixupResolver.reset(new CompilerFixupResolver(this));
  symbolResolver.reset(new CompilerSymbolResolver(this));
  codeOptimizer.reset(new CompilerCodeOptimizer(this));
  expressionEvaluator.reset(new CompilerExpressionEvaluator(this));
  floatConverter.reset(new CompilerFloatConverter(this));
  variableEmitter.reset(new CompilerVariableEmitter(this));
  symbolManager.reset(new SymbolManager());
  resourceManager.reset(new ResourceManager());

  clear();
}

CompilerContext::~CompilerContext() = default;

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

  symbolManager->clear();
  resourceManager->clear();

  while (!forNextStack.empty()) forNextStack.pop();

  if (!temp_str_mark) {
    temp_str_mark.reset(new SymbolNode());
    temp_str_mark->lexeme =
        new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric,
                   "_TEMPSTR_START_", "0");
    temp_str_mark->lexeme->isAbstract = true;
  }

  if (!heap_mark) {
    heap_mark.reset(new SymbolNode());
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
