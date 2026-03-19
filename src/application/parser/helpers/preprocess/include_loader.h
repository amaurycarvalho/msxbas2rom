#ifndef INCLUDE_LOADER_H_INCLUDED
#define INCLUDE_LOADER_H_INCLUDED

#include <string>

#include "lexeme.h"

using namespace std;

class ParserLineEvaluator;
class LexerLineEvaluator;

class IncludeLoader {
 private:
  ParserLineEvaluator* lineEvaluator;

 public:
  explicit IncludeLoader(ParserLineEvaluator* lineEvaluator);
  ~IncludeLoader();

  bool load(shared_ptr<Lexeme> lexeme,
            shared_ptr<LexerLineEvaluator> lexerLine);
  bool load(const string& filename, shared_ptr<LexerLineEvaluator> lexerLine);
};

#endif  // INCLUDE_LOADER_H_INCLUDED
