#include "identifier_state.h"

#include <ctype.h>

LexerLineProcessResult IdentifierState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (c <= ' ' || !context.isIdentifier(c, false)) {
    context.normalizeKeyword();
    context.pushCurrentLexemeAndReset();
    context.rewindCurrentChar();

    return LexerLineProcessResult::Continue;
  }

  context.lexeme->value += toupper(c);
  context.lexeme->name = context.lexeme->value;

  if (context.lexeme->isKeyword()) {
    bool isAnotherKeyword = false;
    for (int ii = context.index + 1;
         ii < context.length && ii < (context.index + 20); ii++) {
      c = context.lexerLine->line[ii];
      context.lexeme->value += toupper(c);
      if (context.lexeme->isKeyword()) {
        isAnotherKeyword = true;
        break;
      }
    }

    context.lexeme->value = context.lexeme->name;

    if (isAnotherKeyword) {
      return LexerLineProcessResult::Continue;
    }

    context.normalizeKeyword();
    context.pushCurrentLexeme();

    if (context.lexeme->value == "REM") {
      return LexerLineProcessResult::Accept;
    }

    context.lexeme = new Lexeme();
  } else {
    // VALTYP: %=2 $=3 !=4 #=8
    if (c == '%')
      context.lexeme->subtype = Lexeme::subtype_numeric;
    else if (c == '$')
      context.lexeme->subtype = Lexeme::subtype_string;
    else if (c == '!')
      context.lexeme->subtype = Lexeme::subtype_single_decimal;
    else if (c == '#')
      context.lexeme->subtype = Lexeme::subtype_double_decimal;
  }

  return LexerLineProcessResult::Continue;
}
