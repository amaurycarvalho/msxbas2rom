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
#include "assignment_evaluator.h"
#include "build_options.h"
#include "expression_evaluator.h"
#include "fswrapper.h"
#include "include_loader.h"
#include "i_parser_processor.h"
#include "lexer.h"
#include "parser_context.h"
#include "parser_statement_strategy_factory.h"
#include "tag_node.h"

using namespace std;

/***
 * @class Parser
 * @brief Parser class specialized as a MSX BASIC syntax tree builder
 */
class Parser : public IParserProcessor {
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

  ParserContext ctx;
  ParserStatementStrategyFactory statementStrategyFactory;
  ExpressionEvaluator exprEval;
  AssignmentEvaluator assignEval;
  IncludeLoader includeLoader;

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
   * @return True, if syntatic analysis success
   */
  bool evalExpressionTokens(LexerLine* expression);

  /***
   * @brief Assignments syntatic analysis
   * @return True, if syntatic analysis success
   */
  bool evalAssignmentTokens(LexerLine* assignment);
  bool processLine(LexerLine* line) override;

  //! Lexeme/Action auxiliary methods

  int gfxOperatorFromLexeme(Lexeme* lexeme);
  Lexeme* coalesceLexeme(Lexeme* lexeme);
  void pushActionNodeRoot(ActionNode* action);
  ActionNode* pushActionFromLexemeNode(Lexeme* lexeme);
  void popActionNodeRoot();

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
