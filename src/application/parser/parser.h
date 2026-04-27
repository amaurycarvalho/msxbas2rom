/***
 * @file parser.h
 * @brief Parser class header specialized as a MSX-BASIC syntax tree builder
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

class Logger;
class Lexer;
class BuildOptions;
class TagNode;
class Lexeme;
class ParserStatementStrategyFactory;

class ParserContext;

using namespace std;

/***
 * @class Parser
 * @brief Parser class specialized as a MSX-BASIC syntax tree builder
 */
class Parser {
 private:
  shared_ptr<ParserStatementStrategyFactory> statementStrategyFactory;

  shared_ptr<ParserContext> ctx;

 public:
  /***
   * @brief Perform a full syntatic analysis on the tags list
   * @return True, if syntatic analysis success
   */
  bool evaluate(shared_ptr<Lexer> lexer);

  shared_ptr<Logger> getLogger();

  int getLineNumber() const;

  vector<shared_ptr<TagNode>>& getTags();
  const vector<shared_ptr<TagNode>>& getTags() const;

  vector<shared_ptr<Lexeme>>& getSymbolList();
  const vector<shared_ptr<Lexeme>>& getSymbolList() const;

  vector<shared_ptr<Lexeme>>& getDatas();
  const vector<shared_ptr<Lexeme>>& getDatas() const;

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
  bool getHasFileSupport() const;

  int getResourceCount() const;

  shared_ptr<Lexer> getLexer() const;
  shared_ptr<BuildOptions> getOpts() const;

  /***
   * @brief Return all tags and it's syntax tree as a string
   */
  string toString();

  /***
   * @brief Parse class constructor, specialized as a
   * MSX-BASIC syntax tree builder
   */
  Parser();
  ~Parser();
};

#endif  // PARSE_H_INCLUDED
