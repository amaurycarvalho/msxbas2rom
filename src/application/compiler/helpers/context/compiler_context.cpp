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
#include "symbol_export_context.h"
#include "symbol_manager.h"

CompilerContext::CompilerContext() {
  logger = make_shared<Logger>();
  symbolManager = make_shared<SymbolManager>();
  resourceManager = make_shared<ResourceManager>();

  temp_str_mark = make_shared<SymbolNode>();
  if (temp_str_mark) {
    temp_str_mark->lexeme =
        make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_numeric,
                            "_TEMPSTR_START_", "0");
    temp_str_mark->lexeme->isAbstract = true;
  }

  heap_mark = make_shared<SymbolNode>();
  if (heap_mark) {
    heap_mark->lexeme = make_shared<Lexeme>(
        Lexeme::type_identifier, Lexeme::subtype_numeric, "_HEAP_", "0");
    heap_mark->lexeme->isAbstract = true;
  }

  clear();
}

CompilerContext::~CompilerContext() = default;

void CompilerContext::setHelpers(shared_ptr<CompilerContext> context) {
  if (context) {
    evaluator = make_shared<CompilerEvaluator>(context);
    codeHelper = make_shared<CompilerCodeHelper>(context);
    fixupResolver = make_shared<CompilerFixupResolver>(context);
    symbolResolver = make_shared<CompilerSymbolResolver>(context);
    codeOptimizer = make_shared<CompilerCodeOptimizer>(context);
    expressionEvaluator = make_shared<CompilerExpressionEvaluator>(context);
    floatConverter = make_shared<CompilerFloatConverter>(context);
    variableEmitter = make_shared<CompilerVariableEmitter>(context);
  }
}

void CompilerContext::clear() {
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
  enableRomBasicMark = nullptr;
  disableRomBasicMark = nullptr;
  drawStmtMark = nullptr;
  ioRedirectMark = nullptr;
  ioScreenMark = nullptr;

  symbols.clear();
  fixes.clear();

  symbolManager->context->clear();
  resourceManager->clear();

  while (!forNextStack.empty()) forNextStack.pop();
}

bool CompilerContext::containErrors() {
  return logger->containErrors();
}

void CompilerContext::syntaxError() {
  syntaxError("Syntax error");
}

void CompilerContext::syntaxError(string msg) {
  compiled = false;
  logger->error(msg);
}
