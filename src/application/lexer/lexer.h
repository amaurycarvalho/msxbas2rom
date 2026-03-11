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

class LexerLine;
class BuildOptions;
class Logger;

using namespace std;

/***
 * @class Lexer
 * @brief Lexer class specialized as a MSX BASIC lexical analyzer
 */
class Lexer {
 public:
  vector<LexerLine*> lines;
  BuildOptions* opts;
  unique_ptr<Logger> logger;

  void clear();
  bool load(string filename);
  bool load(BuildOptions* opts);
  bool evaluate();
  string toString();

  Lexer();
  ~Lexer();
};

#endif  // LEX_H_INCLUDED
