#ifndef LEXER_KEYWORD_STATE_H_INCLUDED
#define LEXER_KEYWORD_STATE_H_INCLUDED

#include "lexer_line_state.h"

class KeywordState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_KEYWORD_STATE_H_INCLUDED
