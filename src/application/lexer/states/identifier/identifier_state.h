#ifndef LEXER_IDENTIFIER_STATE_H_INCLUDED
#define LEXER_IDENTIFIER_STATE_H_INCLUDED

#include "lexer_line_state.h"

class IdentifierState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_IDENTIFIER_STATE_H_INCLUDED
