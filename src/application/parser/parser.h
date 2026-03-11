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

#include <memory>
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

class Logger;

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

  Lexer* lexer;
  BuildOptions* opts;

 public:
  /***
   * @brief Perform a full syntatic analysis on the tags list
   * @return True, if syntatic analysis success
   */
  bool evaluate(Lexer* lexer);

  Logger* getLogger();

  int getLineNumber() const;

  vector<TagNode*>& getTags();
  const vector<TagNode*>& getTags() const;

  vector<Lexeme*>& getSymbolList();
  const vector<Lexeme*>& getSymbolList() const;

  vector<Lexeme*>& getDatas();
  const vector<Lexeme*>& getDatas() const;

  bool getHasTraps() const;
  bool getHasDefusr() const;
  bool getHasData() const;
  bool getHasIData() const;
  bool getHasPlay() const;
  bool getHasInput() const;
  bool getHasFont() const;
  bool getHasMtf() const;
  bool getHasPt3() const;
  bool getHasAkm() const;
  bool getHasResourceRestore() const;

  int getResourceCount() const;

  Lexer* getLexer() const;
  BuildOptions* getOpts() const;

  /***
   * @brief Return all tags and it's syntax tree as a string
   */
  string toString();

  /***
   * @brief Parse class constructor, specialized as a
   * MSX BASIC syntax tree builder
   */
  Parser();
  ~Parser();
};

#endif  // PARSE_H_INCLUDED
