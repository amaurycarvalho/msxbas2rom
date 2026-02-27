#ifndef LEXER_LITERAL_STATE_H_INCLUDED
#define LEXER_LITERAL_STATE_H_INCLUDED

#include "lexer_line_state.h"

class LiteralState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_LITERAL_STATE_H_INCLUDED
