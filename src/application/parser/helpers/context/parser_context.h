/***
 * @file parser_context.h
 * @brief ParserContext class header
 * @author Amaury Carvalho (2019-2025)
 */

#ifndef PARSER_CONTEXT_H_INCLUDED
#define PARSER_CONTEXT_H_INCLUDED

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "action_node.h"
#include "lexeme.h"
#include "lexer_line.h"
#include "tag_node.h"

class Logger;

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
  unique_ptr<Lexeme> lex_null, lex_index, lex_empty_string;

  stack<ActionNode*> actionStack;
  stack<Lexeme*> expressionList;

  int deftbl[26];

  bool eval_expr_error, line_comment;
  unique_ptr<Logger> logger;

  int lineNumber;
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

  int gfxOperatorCode(Lexeme* lexeme);
  ActionNode* pushActionFromLexeme(Lexeme* lexeme);
  void pushStackFromLexeme(Lexeme* lexeme);
  void pushActionRoot(ActionNode* action);
  void popActionRoot();
  Lexeme* coalesceSymbols(Lexeme* lexeme);
};

#endif  // PARSER_CONTEXT_H_INCLUDED
