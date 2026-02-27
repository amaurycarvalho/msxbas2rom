#ifndef LEXER_LINE_STATE_FACTORY_H_INCLUDED
#define LEXER_LINE_STATE_FACTORY_H_INCLUDED

#include "comment_state.h"
#include "identifier_state.h"
#include "keyword_state.h"
#include "literal_state.h"
#include "operator_state.h"
#include "separator_state.h"
#include "unknown_state.h"

class LexerLineStateFactory {
 private:
  UnknownState unknownState;
  LiteralState literalState;
  IdentifierState identifierState;
  KeywordState keywordState;
  OperatorState operatorState;
  SeparatorState separatorState;
  CommentState commentState;

 public:
  ILexerLineState* getState(Lexeme::LexemeType type);
};

#endif  // LEXER_LINE_STATE_FACTORY_H_INCLUDED
