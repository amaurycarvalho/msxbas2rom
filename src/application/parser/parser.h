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
  /***
   * @brief Extract a phrase from current line and evaluate it
   * @note Check for LINE NUMBERS and DIRECTIVES
   * @return True, if syntatic analysis success
   */
  bool eval_line(LexerLine* lexerLine);

  /***
   * @brief Phrase syntatic analysis
   * @note Check if the current phrase it's a statement, expression or
   * assignment
   * @return True, if syntatic analysis success
   */
  bool eval_phrase(LexerLine* phrase);

  /***
   * @brief Statement syntatic analysis
   * @note Check for the correct statement strategy and apply it
   * @return True, if syntatic analysis success
   */
  bool eval_statement(LexerLine* statement);

  /***
   * @brief Assignments syntatic analysis
   * @return True, if syntatic analysis success
   */
  bool eval_assignment(LexerLine* assignment);

  /***
   * @brief Expressions syntatic analysis
   * @note Math/Logical/String expressions parser
   * @return True, if syntatic analysis success
   */
  bool eval_expression(LexerLine* parm);

  /***
   * @brief Expressions stack helper (push)
   * @return True, if success
   */
  bool eval_expression_push(LexerLine* parm);

  /***
   * @brief Expressions stack helper (pop)
   */
  void eval_expression_pop(int n);

  /***
   * @defgroup StatementsParsingGroup
   * @brief Statements parsing group
   * @{
   */

  bool eval_cmd_put(LexerLine* statement);
  bool eval_cmd_base(LexerLine* statement);
  bool eval_cmd_vdp(LexerLine* statement);
  bool eval_cmd_color(LexerLine* statement);
  bool eval_cmd_if(LexerLine* statement, int level);
  bool eval_cmd_for(LexerLine* statement);
  bool eval_cmd_next(LexerLine* statement);
  bool eval_cmd_pset(LexerLine* statement);
  bool eval_cmd_line(LexerLine* statement);
  bool eval_cmd_circle(LexerLine* statement);
  bool eval_cmd_paint(LexerLine* statement);
  bool eval_cmd_copy(LexerLine* statement);
  bool eval_cmd_screen(LexerLine* statement);
  bool eval_cmd_set(LexerLine* statement);
  bool eval_cmd_set_sprite_colpattra(LexerLine* statement);
  bool eval_cmd_get(LexerLine* statement);
  bool eval_cmd_on(LexerLine* statement);
  bool eval_cmd_interval(LexerLine* statement);
  bool eval_cmd_stop(LexerLine* statement);
  bool eval_cmd_key(LexerLine* statement);
  bool eval_cmd_strig(LexerLine* statement);
  bool eval_cmd_sprite(LexerLine* statement);
  bool eval_cmd_sprite_load(LexerLine* statement);
  bool eval_cmd_data(LexerLine* statement, Lexeme::LexemeSubType subtype);
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
   * @brief Parser context getter for strategies use
   * @return context object
   */
  ParserContext& getContext();
  const ParserContext& getContext() const;

  /***
   * @brief Perform a full syntatic analysis on the tags list
   * @return True, if syntatic analysis success
   */
  bool evaluate(Lexer* lexer);

  /***
   * @brief Phrase syntatic analysis
   * @note Check if the current phrase it's a statement, expression or
   * assignment
   * @return True, if syntatic analysis success
   */
  bool evalPhraseTokens(LexerLine* phrase);

  /***
   * @brief Expressions syntatic analysis
   * @note Math/Logical/String expressions parser
   * @return True, if syntatic analysis success
   */
  bool evalExpressionTokens(LexerLine* parm);

  /***
   * @brief Assignments syntatic analysis
   * @return True, if syntatic analysis success
   */
  bool evalAssignmentTokens(LexerLine* assignment);

  //! Lexeme/Action auxiliary methods

  int gfxOperatorFromLexeme(Lexeme* lexeme);
  Lexeme* coalesceLexeme(Lexeme* lexeme);
  void pushActionNodeRoot(ActionNode* action);
  ActionNode* pushActionFromLexemeNode(Lexeme* lexeme);
  void popActionNodeRoot();

  //! Refactoring transitory code

  bool evalCmdData(LexerLine* statement, Lexeme::LexemeSubType subtype);
  bool evalCmdScreen(LexerLine* statement);
  bool evalCmdSprite(LexerLine* statement);
  bool evalCmdBase(LexerLine* statement);
  bool evalCmdVdp(LexerLine* statement);
  bool evalCmdPut(LexerLine* statement);
  bool evalCmdSet(LexerLine* statement);
  bool evalCmdGet(LexerLine* statement);
  bool evalCmdOn(LexerLine* statement);
  bool evalCmdInterval(LexerLine* statement);
  bool evalCmdStop(LexerLine* statement);
  bool evalCmdKey(LexerLine* statement);
  bool evalCmdStrig(LexerLine* statement);
  bool evalCmdColor(LexerLine* statement);
  bool evalCmdCmd(LexerLine* statement);
  bool evalCmdOpen(LexerLine* statement);
  bool evalCmdClose(LexerLine* statement);
  bool evalCmdMaxfiles(LexerLine* statement);
  bool evalCmdIf(LexerLine* statement);
  bool evalCmdFor(LexerLine* statement);
  bool evalCmdNext(LexerLine* statement);
  bool evalCmdPset(LexerLine* statement);
  bool evalCmdLine(LexerLine* statement);
  bool evalCmdCircle(LexerLine* statement);
  bool evalCmdPaint(LexerLine* statement);
  bool evalCmdCopy(LexerLine* statement);
  bool evalCmdSetSpriteColpattra(LexerLine* statement);

  /***
   * @brief Return all tags and it's syntax tree as a string
   */
  string toString();

  /***
   * @brief Return the invalid tag node as a string
   */
  string errorToString();

  /***
   * @brief Parse class constructor, specialized as a
   * MSX BASIC syntax tree builder
   */
  Parser();
  ~Parser();
};

#endif  // PARSE_H_INCLUDED
