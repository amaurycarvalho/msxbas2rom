#ifndef LEXER_UNKNOWN_STATE_H_INCLUDED
#define LEXER_UNKNOWN_STATE_H_INCLUDED

#include "lexer_line_state.h"

class UnknownState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_UNKNOWN_STATE_H_INCLUDED
