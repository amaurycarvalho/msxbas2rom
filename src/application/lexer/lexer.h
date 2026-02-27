/***
 * @file lexer.h
 * @brief MSX BASIC lexer class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   [Lexical analysis](https://en.wikipedia.org/wiki/Lexical_analysis)
 */

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include <string>
#include <vector>

#include "fswrapper.h"
#include "lexer_line.h"
#include "options.h"

using namespace std;

/***
 * @class Lexer
 * @brief Lexer class specialized as a MSX BASIC lexical analyzer
 */
class Lexer {
 public:
  int lineNo = 0;
  vector<LexerLine*> lines;
  string errorMessage;
  BuildOptions* opts;

  void clear();
  bool load(string filename);
  bool load(BuildOptions* opts);
  bool evaluate();
  void print();
  void error();

  Lexer();
};

#endif  // LEX_H_INCLUDED
