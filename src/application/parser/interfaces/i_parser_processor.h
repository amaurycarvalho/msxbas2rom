#ifndef I_PARSER_PROCESSOR_H_INCLUDED
#define I_PARSER_PROCESSOR_H_INCLUDED

#include "lexer_line.h"

class IParserProcessor {
 public:
  virtual ~IParserProcessor() {}

  virtual bool processLine(LexerLine* line) = 0;
};

#endif  // I_PARSER_PROCESSOR_H_INCLUDED
