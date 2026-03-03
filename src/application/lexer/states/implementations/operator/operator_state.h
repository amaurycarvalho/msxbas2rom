#ifndef LEXER_OPERATOR_STATE_H_INCLUDED
#define LEXER_OPERATOR_STATE_H_INCLUDED

#include "lexer_line_state.h"

class OperatorState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_OPERATOR_STATE_H_INCLUDED
