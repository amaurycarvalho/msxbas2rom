#include "lexer_line_state.h"

#include <ctype.h>

LexerLineStateContext::LexerLineStateContext(LexerLine* lexerLine)
    : lexerLine(lexerLine),
      index(0),
      length(lexerLine->line.length()),
      hexa(false),
      current(0),
      lexeme(new Lexeme()) {}

bool LexerLineStateContext::hasNextChar() const {
  return index + 1 < length;
}

char LexerLineStateContext::peekNextChar() const {
  return hasNextChar() ? lexerLine->line[index + 1] : '\0';
}

string LexerLineStateContext::restFromNext() const {
  return lexerLine->line.substr(index + 1);
}

bool LexerLineStateContext::isNumeric(char c) const {
  return (c >= '0' && c <= '9');
}

bool LexerLineStateContext::isDecimal(char c) const {
  return isNumeric(c) || c == '.';
}

bool LexerLineStateContext::isHexDecimal(char c) const {
  return isNumeric(c) || c == 'B' || c == 'b' || c == 'h' || c == 'H' ||
         c == 'o' || c == 'O' || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool LexerLineStateContext::isSeparator(char c) const {
  return (c == ':' || c == '(' || c == ')' || c == '{' || c == '}' ||
          c == ',' || c == ';');
}

bool LexerLineStateContext::isOperator(char c) const {
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
          c == '<' || c == '>' || c == '^' ||
          c == '\\');  //|| c == '\''); remark quote symbol
}

bool LexerLineStateContext::isIdentifier(char c, bool start) const {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
          ((c == '%' || c == '$' || c == '!' || c == '#' ||
            (c >= '0' && c <= '9')) &&
           !start));
}

bool LexerLineStateContext::isComment(char c) const {
  return false;
}

void LexerLineStateContext::pushCurrentLexeme() {
  lexerLine->addLexeme(lexeme);
}

void LexerLineStateContext::pushCurrentLexemeAndReset() {
  pushCurrentLexeme();
  lexeme = new Lexeme();
}

void LexerLineStateContext::rewindCurrentChar() {
  if (current > ' ') index--;
}

void LexerLineStateContext::normalizeKeyword() {
  if (lexeme->isKeyword()) {
    lexeme->type = Lexeme::type_keyword;
    lexeme->subtype = Lexeme::subtype_any;
    if (lexeme->isBooleanOperator()) {
      lexeme->type = Lexeme::type_operator;
      lexeme->subtype = Lexeme::subtype_boolean_operator;
    } else if (lexeme->isFunction()) {
      lexeme->subtype = Lexeme::subtype_function;
    }
  }
}
