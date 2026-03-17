#ifndef INCLUDE_LOADER_H_INCLUDED
#define INCLUDE_LOADER_H_INCLUDED

#include <string>

#include "lexeme.h"

using namespace std;

class ParserLineEvaluator;
class LexerLineEvaluator;

class IncludeLoader {
 public:
  IncludeLoader(ParserLineEvaluator& lineEvaluator);

  bool load(shared_ptr<Lexeme> lexeme, LexerLineEvaluator* lexerLine);
  bool load(const string& filename, LexerLineEvaluator* lexerLine);

 private:
  ParserLineEvaluator& lineEvaluator;
};

#endif  // INCLUDE_LOADER_H_INCLUDED
