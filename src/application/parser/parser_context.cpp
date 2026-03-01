/***
 * @file parser_context.cpp
 * @brief ParserContext class code
 * @author Amaury Carvalho (2019-2025)
 */

#include "parser_context.h"

ParserContext::ParserContext() {
  lex_null = new Lexeme(Lexeme::type_literal, Lexeme::subtype_null, "NULL");
  lex_empty_string =
      new Lexeme(Lexeme::type_literal, Lexeme::subtype_string, "");
  lex_index =
      new Lexeme(Lexeme::type_keyword, Lexeme::subtype_numeric, "INDEX");

  reset();
}

ParserContext::~ParserContext() {
  if (lex_null) delete lex_null;
  if (lex_empty_string) delete lex_empty_string;
  if (lex_index) delete lex_index;
}

void ParserContext::reset() {
  int i;

  tag = 0;
  actionRoot = 0;
  error_line = 0;
  resourceCount = 0;

  lineNo = 0;
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
  error_message = "";

  for (i = 0; i < 26; i++) deftbl[i] = 0;

  tags.clear();
  symbolList.clear();
  datas.clear();

  while (!actionStack.empty()) actionStack.pop();
  while (!expressionList.empty()) expressionList.pop();
}
