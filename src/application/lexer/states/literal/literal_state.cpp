#include "literal_state.h"

LexerLineProcessResult LiteralState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (context.lexeme->subtype == Lexeme::subtype_string) {
    context.lexeme->value += c;
    if (c == '"') {
      context.pushCurrentLexemeAndReset();
    }
    return LexerLineProcessResult::Continue;
  }

  if (context.hexa) {
    if (context.isDecimal(c) || context.isHexDecimal(c)) {
      context.lexeme->value += c;
    } else {
      context.pushCurrentLexemeAndReset();
      context.rewindCurrentChar();
    }
    return LexerLineProcessResult::Continue;
  }

  if (context.isDecimal(c)) {
    context.lexeme->value += c;
    if (context.lexeme->subtype == Lexeme::subtype_double_decimal && c == '.') {
      context.lexeme->type = Lexeme::type_unknown;
      context.lexeme->subtype = Lexeme::subtype_any;
      context.pushCurrentLexeme();
      return LexerLineProcessResult::Reject;
    }

    if (c == '.') {
      context.lexeme->subtype = Lexeme::subtype_double_decimal;
    }

    if (context.lexeme->subtype == Lexeme::subtype_numeric) {
      if (context.lexeme->value.size() > 5 ||
          (context.lexeme->value.size() == 5 &&
           context.lexeme->value > "32767")) {
        context.lexeme->subtype = Lexeme::subtype_single_decimal;
      }
    }

    if (context.lexeme->subtype == Lexeme::subtype_single_decimal) {
      if (context.lexeme->value.size() > 6) {
        context.lexeme->subtype = Lexeme::subtype_double_decimal;
      }
    }

    return LexerLineProcessResult::Continue;
  }

  if (c == '%') {
    context.lexeme->subtype = Lexeme::subtype_numeric;
  } else if (c == '#') {
    context.lexeme->subtype = Lexeme::subtype_single_decimal;
  } else if (c == '!') {
    context.lexeme->subtype = Lexeme::subtype_double_decimal;
  } else {
    context.pushCurrentLexemeAndReset();
    context.rewindCurrentChar();
  }

  return LexerLineProcessResult::Continue;
}
