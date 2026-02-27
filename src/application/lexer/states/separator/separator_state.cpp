#include "separator_state.h"

LexerLineProcessResult SeparatorState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (c <= ' ' || !context.isSeparator(c)) {
    context.pushCurrentLexemeAndReset();
    context.rewindCurrentChar();
  } else {
    context.lexeme->value += c;
  }

  return LexerLineProcessResult::Continue;
}
