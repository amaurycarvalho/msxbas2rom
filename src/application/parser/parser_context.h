/***
 * @file parser_context.h
 * @brief ParserContext class header
 * @author Amaury Carvalho (2019-2025)
 */

#ifndef PARSER_CONTEXT_H_INCLUDED
#define PARSER_CONTEXT_H_INCLUDED

#include <stack>
#include <string>
#include <vector>

#include "action_node.h"
#include "lexer_line.h"
#include "tag_node.h"

using namespace std;

/***
 * @class ParserContext
 * @brief Mutable parser execution state shared across parser methods.
 */
class ParserContext {
 public:
  TagNode* tag;
  ActionNode* actionRoot;
  LexerLine* error_line;
  Lexeme *lex_null, *lex_index, *lex_empty_string;

  stack<ActionNode*> actionStack;
  stack<Lexeme*> expressionList;

  int deftbl[26];

  bool eval_expr_error, line_comment;
  string error_message;

  int lineNo;
  vector<TagNode*> tags;
  vector<Lexeme*> symbolList;
  vector<Lexeme*> datas;

  bool has_traps, has_defusr, has_data, has_idata;
  bool has_play, has_input, has_font, has_mtf;
  bool has_pt3, has_akm, has_resource_restore;
  int resourceCount;

  ParserContext();
  ~ParserContext();

  void reset();
};

#endif  // PARSER_CONTEXT_H_INCLUDED
