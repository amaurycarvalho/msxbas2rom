/***
 * @file parser.h
 * @brief Parser class header specialized as a MSX BASIC syntax tree builder
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Parsing
 *   https://en.wikipedia.org/wiki/Recursive_descent_parser
 *   https://www.strchr.com/expression_evaluator
 *   https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */

#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <array>
#include <stack>
#include <string>
#include <vector>

#include "action_node.h"
#include "build_options.h"
#include "fswrapper.h"
#include "lexer.h"
#include "parser_context.h"
#include "parser_statement_strategy_factory.h"
#include "tag_node.h"

using namespace std;

/***
 * @class Parser
 * @brief Parser class specialized as a MSX BASIC syntax tree builder
 */
class Parser {
 private:
  bool eval_line(LexerLine* lexerLine);
  bool eval_phrase(LexerLine* phrase);
  bool eval_statement(LexerLine* statement);
  bool eval_assignment(LexerLine* assignment);
  bool eval_expression(LexerLine* parm);
  bool eval_expression_push(LexerLine* parm);
  void eval_expression_pop(int n);

  /***
   * @defgroup StatementsParsingGroup
   * @brief Statements parsing group
   * @{
   */

  bool eval_cmd_generic(LexerLine* statement);
  bool eval_cmd_let(LexerLine* statement);
  bool eval_cmd_dim(LexerLine* statement);
  bool eval_cmd_def(LexerLine* statement, int vartype);
  bool eval_cmd_def_usr(LexerLine* statement);
  bool eval_cmd_print(LexerLine* statement);
  bool eval_cmd_input(LexerLine* statement);
  bool eval_cmd_line_input(LexerLine* statement);
  bool eval_cmd_put(LexerLine* statement);
  bool eval_cmd_put_sprite(LexerLine* statement);
  bool eval_cmd_put_tile(LexerLine* statement);
  bool eval_cmd_base(LexerLine* statement);
  bool eval_cmd_vdp(LexerLine* statement);
  bool eval_cmd_time(LexerLine* statement);
  bool eval_cmd_color(LexerLine* statement);
  bool eval_cmd_color_rgb(LexerLine* statement);
  bool eval_cmd_color_sprite(LexerLine* statement);
  bool eval_cmd_if(LexerLine* statement, int level);
  bool eval_cmd_for(LexerLine* statement);
  bool eval_cmd_next(LexerLine* statement);
  bool eval_cmd_pset(LexerLine* statement);
  bool eval_cmd_line(LexerLine* statement);
  bool eval_cmd_circle(LexerLine* statement);
  bool eval_cmd_paint(LexerLine* statement);
  bool eval_cmd_copy(LexerLine* statement);
  bool eval_cmd_screen(LexerLine* statement);
  bool eval_cmd_screen_copy(LexerLine* statement);
  bool eval_cmd_screen_paste(LexerLine* statement);
  bool eval_cmd_screen_scroll(LexerLine* statement);
  bool eval_cmd_screen_load(LexerLine* statement);
  bool eval_cmd_screen_on(LexerLine* statement);
  bool eval_cmd_screen_off(LexerLine* statement);
  bool eval_cmd_set(LexerLine* statement);
  bool eval_cmd_set_adjust(LexerLine* statement);
  bool eval_cmd_set_tile(LexerLine* statement);
  bool eval_cmd_set_tile_colpat(LexerLine* statement);
  bool eval_cmd_set_sprite(LexerLine* statement);
  bool eval_cmd_set_sprite_colpattra(LexerLine* statement);
  bool eval_cmd_get(LexerLine* statement);
  bool eval_cmd_get_tile(LexerLine* statement);
  bool eval_cmd_get_sprite(LexerLine* statement);
  bool eval_cmd_on(LexerLine* statement);
  bool eval_cmd_on_goto_gosub(LexerLine* statement);
  bool eval_cmd_on_error(LexerLine* statement);
  bool eval_cmd_on_interval(LexerLine* statement);
  bool eval_cmd_on_key(LexerLine* statement);
  bool eval_cmd_on_sprite(LexerLine* statement);
  bool eval_cmd_on_stop(LexerLine* statement);
  bool eval_cmd_on_strig(LexerLine* statement);
  bool eval_cmd_interval(LexerLine* statement);
  bool eval_cmd_stop(LexerLine* statement);
  bool eval_cmd_key(LexerLine* statement);
  bool eval_cmd_strig(LexerLine* statement);
  bool eval_cmd_sprite(LexerLine* statement);
  bool eval_cmd_sprite_load(LexerLine* statement);
  bool eval_cmd_data(LexerLine* statement, Lexeme::LexemeSubType subtype);
  bool eval_cmd_call(LexerLine* statement);
  bool eval_cmd_cmd(LexerLine* statement);
  bool eval_cmd_open(LexerLine* statement);
  bool eval_cmd_close(LexerLine* statement);
  bool eval_cmd_maxfiles(LexerLine* statement);

  /***
   * @remark End of StatementsParsingGroup
   * @}
   */

  bool loadInclude(Lexeme* lexeme);
  int gfxOperatorCode(Lexeme* lexeme);

  int getOperatorPrecedence(Lexeme* lexeme);
  int getOperatorParmCount(Lexeme* lexeme);
  ActionNode* pushActionFromLexeme(Lexeme* lexeme);
  void pushStackFromLexeme(Lexeme* lexeme);

  void pushActionRoot(ActionNode* action);
  void popActionRoot();

  Lexeme* coalesceSymbols(Lexeme* lexeme);

  ParserContext ctx;
  ParserStatementStrategyFactory statementStrategyFactory;

  TagNode*& tag;
  ActionNode*& actionRoot;
  LexerLine*& error_line;
  Lexeme*& lex_null;
  Lexeme*& lex_index;
  Lexeme*& lex_empty_string;

  stack<ActionNode*>& actionStack;
  stack<Lexeme*>& expressionList;
  int (&deftbl)[26];

  bool& eval_expr_error;
  bool& line_comment;
  string& error_message;

 public:
  int& lineNo;
  //! @brief abstract syntax tree in list form implementation
  vector<TagNode*>& tags;
  //! @brief symbols list coalesced
  vector<Lexeme*>& symbolList;
  vector<Lexeme*>& datas;

  bool& has_traps;
  bool& has_defusr;
  bool& has_data;
  bool& has_idata;
  bool& has_play;
  bool& has_input;
  bool& has_font;
  bool& has_mtf;
  bool& has_pt3;
  bool& has_akm;
  bool& has_resource_restore;
  int& resourceCount;

  Lexer* lexer;
  BuildOptions* opts;

  /***
   * @brief Perform a syntatic analysis on the tags list
   * @return True, if syntatic analysis success
   */
  bool evaluate(Lexer* lexer);

  /***
   * @brief Parser context getter
   */
  ParserContext& getContext();
  const ParserContext& getContext() const;
  bool evalCmdGeneric(LexerLine* statement);
  bool evalCmdLet(LexerLine* statement);
  bool evalCmdDim(LexerLine* statement);
  bool evalCmdPrint(LexerLine* statement);

  /***
   * @brief Return all tags and it's syntax tree as a string
   */
  string toString();

  /***
   * @brief Return the invalid tag node as a string
   */
  string errorToString();

  /***
   * @brief Parse class constructor, specialized as a MSX BASIC syntax tree
   * builder
   */
  Parser();
  ~Parser();
};

#endif  // PARSE_H_INCLUDED
