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
#include "lexer_line_context.h"
#include "tag_node.h"

class Logger;

using namespace std;

/***
 * @class ParserContext
 * @brief Mutable parser execution state shared across parser methods.
 */
class ParserContext {
 public:
  //! templates
  shared_ptr<Lexeme> lex_null, lex_index, lex_empty_string;
  shared_ptr<Lexeme> lex_rgb, lex_zero;

  //! collections
  stack<shared_ptr<ActionNode>> actionStack;
  stack<shared_ptr<Lexeme>> expressionList;
  vector<shared_ptr<TagNode>> tags;
  vector<shared_ptr<Lexeme>> symbolList;
  vector<shared_ptr<Lexeme>> datas;

  //! flags
  bool eval_expr_error, line_comment;
  bool has_traps, has_defusr, has_data, has_idata;
  bool has_play, has_input, has_font, has_mtf;
  bool has_pt3, has_akm, has_resource_restore;

  //! counters
  int deftbl[26];
  int lineNumber;
  int resourceCount;

  //! helper objects
  unique_ptr<Logger> logger;

  shared_ptr<TagNode> tag;
  shared_ptr<ActionNode> actionRoot;
  LexerLineContext* error_line;

  //! helper methods
  void reset();

  int gfxOperatorCode(shared_ptr<Lexeme> lexeme);
  shared_ptr<ActionNode> pushActionFromLexeme(shared_ptr<Lexeme> lexeme);
  void pushStackFromLexeme(shared_ptr<Lexeme> lexeme);
  void pushActionRoot(shared_ptr<ActionNode> action);
  void popActionRoot();
  shared_ptr<Lexeme> coalesceSymbols(shared_ptr<Lexeme> lexeme);

  ParserContext();
  ~ParserContext();
};

#endif  // PARSER_CONTEXT_H_INCLUDED
