#ifndef INCLUDE_LOADER_H_INCLUDED
#define INCLUDE_LOADER_H_INCLUDED

#include <string>

#include "lexeme.h"

using namespace std;

class ParserLineEvaluator;

class IncludeLoader {
 public:
  IncludeLoader(ParserLineEvaluator& lineEvaluator);

  bool load(Lexeme* lexeme);
  bool load(const string& filename);

 private:
  ParserLineEvaluator& lineEvaluator;
};

#endif  // INCLUDE_LOADER_H_INCLUDED
