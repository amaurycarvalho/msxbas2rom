/***
 * @file lexer_line.cpp
 * @brief MSX BASIC lexer line class implementation
 * @author Amaury Carvalho (2019-2025)
 */

#include "lexer_line.h"

#include "lexer_line_state_factory.h"

/***
 * @name LexerLine class functions
 */

LexerLine::LexerLine() {
  lexemeIndex = 0;
}

void LexerLine::clearLexemes() {
  lexemes.clear();
}

void LexerLine::addLexeme(Lexeme* lexeme) {
  lexemes.push_back(lexeme);
}

void LexerLine::setLexemeBOF() {
  lexemeIndex = -1;
}

Lexeme* LexerLine::getCurrentLexeme() {
  return getLexeme(lexemeIndex);
}

Lexeme* LexerLine::getFirstLexeme() {
  return getLexeme(0);
}

Lexeme* LexerLine::getNextLexeme() {
  return getLexeme(lexemeIndex + 1);
}

Lexeme* LexerLine::getPreviousLexeme() {
  return getLexeme(lexemeIndex - 1);
}

Lexeme* LexerLine::getLastLexeme() {
  return getLexeme(lexemes.size() - 1);
}

Lexeme* LexerLine::getLexeme(int i) {
  if (i >= 0 && i < (int)lexemes.size()) {
    lexemeIndex = i;
    return lexemes[i];
  } else
    return 0;
}

int LexerLine::getLexemeCount() {
  return lexemes.size();
}

void LexerLine::pushLexeme() {
  lexemeStack.push(lexemeIndex);
}

void LexerLine::popLexeme() {
  if (!lexemeStack.empty()) {
    lexemeIndex = lexemeStack.top();
    lexemeStack.pop();
  }
}

void LexerLine::popLexemeDiscarding() {
  lexemeStack.pop();
}

string LexerLine::toString() {
  string out = line;
  for (unsigned int i = 0; i < lexemes.size(); i++) {
    out += lexemes[i]->toString();
  }
  return out;
}

bool LexerLine::evaluate() {
  LexerLineStateFactory stateFactory;
  LexerLineStateContext context(this);

  lexemes.clear();

  for (context.index = 0; context.index < context.length; context.index++) {
    context.current = line[context.index];
    ILexerLineState* state = stateFactory.getState(context.lexeme->type);
    LexerLineProcessResult result = state->handle(context);
    if (result == LexerLineProcessResult::Accept) return true;
    if (result == LexerLineProcessResult::Reject) return false;
  }

  if (context.lexeme->type != Lexeme::type_unknown) {
    context.normalizeKeyword();
    lexemes.push_back(context.lexeme);
  } else {
    delete context.lexeme;
  }

  return true;
}
