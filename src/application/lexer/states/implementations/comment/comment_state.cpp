#include "comment_state.h"

LexerLineProcessResult CommentState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (c <= ' ' || !context.isComment(c)) {
    context.pushCurrentLexemeAndReset();
    context.rewindCurrentChar();
  } else {
    context.lexeme->value += c;
  }

  return LexerLineProcessResult::Continue;
}
