#include "unknown_state.h"

#include <ctype.h>

LexerLineProcessResult UnknownState::handle(LexerLineStateContext& context) {
  char c = context.current;

  if (c <= ' ') {
    return LexerLineProcessResult::Continue;
  } else if (context.isDecimal(c) || c == '"' || c == '&') {
    context.lexeme->type = Lexeme::type_literal;
    context.lexeme->value += c;
    context.hexa = (c == '&');
    if (c == '.')
      context.lexeme->subtype = Lexeme::subtype_double_decimal;
    else if (c == '"')
      context.lexeme->subtype = Lexeme::subtype_string;
    else
      context.lexeme->subtype = Lexeme::subtype_numeric;

    return LexerLineProcessResult::Continue;
  } else if (context.isOperator(c)) {
    context.lexeme->type = Lexeme::type_operator;
    context.lexeme->value += c;
    context.pushCurrentLexemeAndReset();

    return LexerLineProcessResult::Continue;
  } else if (context.isSeparator(c)) {
    context.lexeme->type = Lexeme::type_separator;
    context.lexeme->value += c;
    context.pushCurrentLexemeAndReset();

    return LexerLineProcessResult::Continue;
  } else if (context.isIdentifier(c, true)) {
    context.lexeme->type = Lexeme::type_identifier;
    context.lexeme->subtype =
        Lexeme::subtype_single_decimal;  // default identifier subtype
    context.lexeme->value += toupper(c);
    context.lexeme->name = context.lexeme->value;

    return LexerLineProcessResult::Continue;
  } else if (c == '?' || c == '_') {
    context.lexeme->type = Lexeme::type_identifier;
    context.lexeme->subtype = Lexeme::subtype_any;
    context.lexeme->value += toupper(c);
    context.lexeme->name = context.lexeme->value;

    return LexerLineProcessResult::Continue;
  } else if (c == '#') {
    context.lexeme->type = Lexeme::type_separator;
    context.lexeme->value += c;
    context.pushCurrentLexemeAndReset();

    return LexerLineProcessResult::Continue;
  } else if (c == '\'') {
    context.lexeme->type = Lexeme::type_operator;
    context.lexeme->subtype = Lexeme::subtype_any;
    context.lexeme->value = "\'";
    context.lexeme->name = context.lexeme->value;
    context.pushCurrentLexeme();

    // if xbasic special commands...
    if (context.hasNextChar() && context.peekNextChar() == '#') {
      string s = context.restFromNext();
      // Remove trailing character (like newline) if present
      if (!s.empty()) {
        s.pop_back();
      }
      context.lexeme = new Lexeme(Lexeme::type_comment, Lexeme::subtype_any, s);
      context.pushCurrentLexeme();
    }

    return LexerLineProcessResult::Accept;
  }

  context.lexeme->type = Lexeme::type_unknown;
  context.lexeme->value += c;
  context.pushCurrentLexeme();

  return LexerLineProcessResult::Reject;
}
