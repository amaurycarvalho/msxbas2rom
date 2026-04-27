/***
 * @file parser_context.cpp
 * @brief ParserContext class code
 * @author Amaury Carvalho (2019-2025)
 */

#include "parser_context.h"

#include "action_node.h"
#include "assignment_evaluator.h"
#include "expression_evaluator.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"
#include "parser_line_evaluator.h"
#include "parser_statement_strategy_factory.h"
#include "tag_node.h"

ParserContext::ParserContext() {
  logger = make_shared<Logger>();
  lex_null =
      make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_null, "NULL");
  lex_empty_string =
      make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_string, "");
  lex_index = make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_numeric,
                                  "INDEX");
  lex_rgb =
      make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, "RGB");
  lex_zero =
      make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_numeric, "0");

  reset();
}

ParserContext::~ParserContext() = default;

void ParserContext::setHelpers(
    shared_ptr<ParserContext> context,
    shared_ptr<ParserStatementStrategyFactory> statementStrategyFactory) {
  exprEval = make_shared<ExpressionEvaluator>(context);
  assignEval = make_shared<AssignmentEvaluator>(context, exprEval);
  lineEval = make_shared<ParserLineEvaluator>(context, exprEval, assignEval,
                                              statementStrategyFactory);
}

void ParserContext::reset() {
  int i;

  tag = 0;
  actionRoot = 0;
  error_line = 0;
  resourceCount = 0;

  lineNumber = 0;
  eval_expr_error = false;
  line_comment = false;
  has_traps = false;
  has_defusr = false;
  has_data = false;
  has_idata = false;
  has_play = false;
  has_input = false;
  has_font = false;
  has_pt3 = false;
  has_akm = false;
  has_mtf = false;
  has_resource_restore = false;
  has_file_support = false;

  for (i = 0; i < 26; i++) deftbl[i] = 0;

  tags.clear();
  symbolList.clear();
  datas.clear();

  while (!actionStack.empty()) actionStack.pop();
  while (!expressionList.empty()) expressionList.pop();
}

shared_ptr<Lexeme> ParserContext::coalesceSymbols(shared_ptr<Lexeme> lexeme) {
  shared_ptr<Lexeme> result = lexeme;
  unsigned int i, t = symbolList.size();
  bool ok = false;
  int c;
  char* s;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_identifier) {
      for (i = 0; i < t; i++) {
        result = symbolList[i];
        if (result->type == lexeme->type &&
            result->subtype == lexeme->subtype &&
            result->name == lexeme->name && result->value == lexeme->value) {
          ok = true;
          break;
        }
      }

      if (!ok) {
        symbolList.push_back(lexeme);
        result = lexeme;
      }

      t = lexeme->value.size();

      if (lexeme->type == Lexeme::type_identifier && t) {
        s = (char*)lexeme->value.c_str();
        c = s[t - 1];
        if (c != '$' && c != '#' && c != '!' && c != '%') {
          c = s[0] - 'A';
          if (c >= 0 && c < 26) {
            switch (deftbl[c]) {
              case 2:
                lexeme->subtype = Lexeme::subtype_numeric;
                break;
              case 3:
                lexeme->subtype = Lexeme::subtype_string;
                break;
              case 4:
                lexeme->subtype = Lexeme::subtype_single_decimal;
                break;
              case 8:
                lexeme->subtype = Lexeme::subtype_double_decimal;
                break;
            }
          }
        }
      }
    }
  }

  return result;
}

void ParserContext::pushStackFromLexeme(shared_ptr<Lexeme> lexeme) {
  expressionList.push(lexeme);
}

shared_ptr<ActionNode> ParserContext::pushActionFromLexeme(
    shared_ptr<Lexeme> lexeme) {
  auto actionExpr = make_shared<ActionNode>(lexeme);

  if (lexeme->type == Lexeme::type_operator ||
      lexeme->type == Lexeme::type_separator ||
      (lexeme->type == Lexeme::type_keyword && lexeme->value != "TIME" &&
       lexeme->value != "INKEY" && lexeme->value != "INKEY$" &&
       lexeme->value != "MAXFILES" && lexeme->value != "ERL" &&
       lexeme->value != "ERR") ||
      (lexeme->type == Lexeme::type_identifier && lexeme->parm_count > 0)) {
    pushActionRoot(actionExpr);

  } else
    actionRoot->actions.push_back(actionExpr);

  return actionExpr;
}

int ParserContext::gfxOperatorCode(shared_ptr<Lexeme> lexeme) {
  int result = -1;

  if (lexeme->value == "PSET") {
    result = 0;
  } else if (lexeme->value == "AND") {
    result = 1;
  } else if (lexeme->value == "OR") {
    result = 2;
  } else if (lexeme->value == "XOR") {
    result = 3;
  } else if (lexeme->value == "PRESET") {
    result = 4;
  } else if (lexeme->value == "TPSET") {
    result = 8;
  } else if (lexeme->value == "TAND") {
    result = 9;
  } else if (lexeme->value == "TOR") {
    result = 10;
  } else if (lexeme->value == "TXOR") {
    result = 11;
  } else if (lexeme->value == "TPRESET") {
    result = 12;
  }

  return result;
}

void ParserContext::pushActionRoot(shared_ptr<ActionNode> action) {
  if (actionRoot) {
    actionRoot->actions.push_back(action);
    actionStack.push(actionRoot);
  } else {
    while (!actionStack.empty()) actionStack.pop();
    tag->actions.push_back(action);
  }
  actionRoot = action;
}

void ParserContext::popActionRoot() {
  if (actionStack.empty()) {
    actionRoot = 0;
  } else {
    actionRoot = actionStack.top();
    actionStack.pop();
  }
}
