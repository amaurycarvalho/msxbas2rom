#ifndef INCLUDE_LOADER_H_INCLUDED
#define INCLUDE_LOADER_H_INCLUDED

#include <memory>
#include <string>

#include "lexeme.h"

using namespace std;

class ParserLineEvaluator;
class LexerLineEvaluator;
class SourceResolver;

class IncludeLoader {
 private:
  ParserLineEvaluator* lineEvaluator;
  shared_ptr<SourceResolver> sourceResolver;

 public:
  explicit IncludeLoader(ParserLineEvaluator* lineEvaluator);
  ~IncludeLoader();

  void setSourceResolver(shared_ptr<SourceResolver> resolver);

  bool load(shared_ptr<Lexeme> lexeme,
            shared_ptr<LexerLineEvaluator> lexerLine);
  bool load(const string& filename, shared_ptr<LexerLineEvaluator> lexerLine);
};

#endif  // INCLUDE_LOADER_H_INCLUDED
