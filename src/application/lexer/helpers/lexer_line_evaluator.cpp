/***
 * @file lexer_line_evaluator.cpp
 * @brief MSX BASIC lexer line evaluator implementation
 * @author Amaury Carvalho (2019-2026)
 */

#include "lexer_line_evaluator.h"

#include "lexeme.h"
#include "lexer_line_state_factory.h"

bool LexerLineEvaluator::evaluate() {
  LexerLineStateFactory stateFactory;
  LexerLineStateContext context(this);

  lexemes.clear();

  for (context.index = 0; context.index < context.length; context.index++) {
    context.current = lineText[context.index];
    ILexerLineState* state = stateFactory.getState(context.lexeme->type);
    LexerLineProcessResult result = state->handle(context);
    if (result == LexerLineProcessResult::Accept) return true;
    if (result == LexerLineProcessResult::Reject) return false;
  }

  if (context.lexeme->type != Lexeme::type_unknown) {
    context.normalizeKeyword();
    addLexeme(context.lexeme->clone());
  }

  return true;
}
