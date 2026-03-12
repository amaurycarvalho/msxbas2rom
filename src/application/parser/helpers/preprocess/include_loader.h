#ifndef INCLUDE_LOADER_H_INCLUDED
#define INCLUDE_LOADER_H_INCLUDED

#include <string>

#include "lexeme.h"

using namespace std;

class ParserLineEvaluator;
class LexerLine;

class IncludeLoader {
 public:
  IncludeLoader(ParserLineEvaluator& lineEvaluator);

  bool load(Lexeme* lexeme, LexerLine* lexerLine);
  bool load(const string& filename, LexerLine* lexerLine);

 private:
  ParserLineEvaluator& lineEvaluator;
};

#endif  // INCLUDE_LOADER_H_INCLUDED
