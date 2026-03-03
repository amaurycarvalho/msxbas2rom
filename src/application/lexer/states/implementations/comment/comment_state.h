#ifndef LEXER_COMMENT_STATE_H_INCLUDED
#define LEXER_COMMENT_STATE_H_INCLUDED

#include "lexer_line_state.h"

class CommentState : public ILexerLineState {
 public:
  LexerLineProcessResult handle(LexerLineStateContext& context) override;
};

#endif  // LEXER_COMMENT_STATE_H_INCLUDED
