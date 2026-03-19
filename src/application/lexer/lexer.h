/***
 * @file lexer.h
 * @brief MSX BASIC lexer class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   [Lexical analysis](https://en.wikipedia.org/wiki/Lexical_analysis)
 */

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class LexerLineEvaluator;
class BuildOptions;
class Logger;

using namespace std;

/***
 * @class Lexer
 * @brief Lexer class specialized as a MSX BASIC lexical analyzer
 */
class Lexer {
 private:
  shared_ptr<Logger> logger;

 public:
  vector<shared_ptr<LexerLineEvaluator>> lines;
  shared_ptr<BuildOptions> opts;

  void clear();
  bool load(string filename);
  bool load(shared_ptr<BuildOptions> opts);
  bool evaluate();
  string toString();

  shared_ptr<Logger> getLogger();

  Lexer();
  ~Lexer();
};

#endif  // LEX_H_INCLUDED
