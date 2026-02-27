#include "keyword_state.h"

#include <ctype.h>

LexerLineProcessResult KeywordState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (c <= ' ' || !context.isIdentifier(c, false)) {
    context.pushCurrentLexemeAndReset();
    context.rewindCurrentChar();
  } else {
    context.lexeme->value += toupper(c);
  }

  return LexerLineProcessResult::Continue;
}
