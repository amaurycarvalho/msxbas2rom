#include "lexer_line_state_factory.h"

ILexerLineState* LexerLineStateFactory::getState(Lexeme::LexemeType type) {
  switch (type) {
    case Lexeme::type_unknown:
      return &unknownState;
    case Lexeme::type_literal:
      return &literalState;
    case Lexeme::type_identifier:
      return &identifierState;
    case Lexeme::type_keyword:
      return &keywordState;
    case Lexeme::type_operator:
      return &operatorState;
    case Lexeme::type_separator:
      return &separatorState;
    case Lexeme::type_comment:
      return &commentState;
    default:
      return &unknownState;
  }
}
