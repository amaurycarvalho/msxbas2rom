#include "operator_state.h"

LexerLineProcessResult OperatorState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (c <= ' ' || !context.isOperator(c)) {
    context.pushCurrentLexemeAndReset();
    context.rewindCurrentChar();
  } else {
    context.lexeme->value += c;
  }

  return LexerLineProcessResult::Continue;
}
