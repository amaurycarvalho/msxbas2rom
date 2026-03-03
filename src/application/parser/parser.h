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

#include <string>
#include <vector>

#include "action_node.h"
#include "assignment_evaluator.h"
#include "build_options.h"
#include "expression_evaluator.h"
#include "lexer.h"
#include "parser_context.h"
#include "parser_line_evaluator.h"
#include "parser_statement_strategy_factory.h"
#include "tag_node.h"

using namespace std;

/***
 * @class Parser
 * @brief Parser class specialized as a MSX BASIC syntax tree builder
 */
class Parser {
 private:
  ParserContext ctx;
  ParserStatementStrategyFactory statementStrategyFactory;
  ExpressionEvaluator exprEval;
  AssignmentEvaluator assignEval;
  ParserLineEvaluator lineEval;

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
   * @brief Perform a full syntatic analysis on the tags list
   * @return True, if syntatic analysis success
   */
  bool evaluate(Lexer* lexer);

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
