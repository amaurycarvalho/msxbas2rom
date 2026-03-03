#ifndef INCLUDE_LOADER_H_INCLUDED
#define INCLUDE_LOADER_H_INCLUDED

#include <string>

#include "i_parser_processor.h"
#include "lexeme.h"

using namespace std;

class IncludeLoader {
 public:
  IncludeLoader(IParserProcessor& parserProcessor);

  bool load(Lexeme* lexeme);
  bool load(const string& filename);

 private:
  IParserProcessor& parserProcessor;
};

#endif  // INCLUDE_LOADER_H_INCLUDED
