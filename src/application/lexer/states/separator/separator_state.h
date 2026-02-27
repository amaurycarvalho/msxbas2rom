#ifndef LEXER_SEPARATOR_STATE_H_INCLUDED
#define LEXER_SEPARATOR_STATE_H_INCLUDED

#include "lexer_line_state.h"

class SeparatorState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_SEPARATOR_STATE_H_INCLUDED
