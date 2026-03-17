/***
 * @file lexer_line.cpp
 * @brief MSX BASIC lexer line class implementation
 * @author Amaury Carvalho (2019-2025)
 */

#include "lexer_line.h"

#include "lexeme.h"
#include "lexer_line_state_factory.h"

/***
 * @name LexerLine class functions
 */

LexerLine::LexerLine() {
  lexemeIndex = 0;
}

LexerLine::~LexerLine() = default;

void LexerLine::clearLexemes() {
  lexemes.clear();
}

void LexerLine::addLexeme(shared_ptr<Lexeme> lexeme) {
  lexemes.emplace_back(lexeme);
}

void LexerLine::setLexemeBOF() {
  lexemeIndex = -1;
}

shared_ptr<Lexeme> LexerLine::getCurrentLexeme() {
  return getLexeme(lexemeIndex);
}

shared_ptr<Lexeme> LexerLine::getFirstLexeme() {
  return getLexeme(0);
}

shared_ptr<Lexeme> LexerLine::getNextLexeme() {
  return getLexeme(lexemeIndex + 1);
}

shared_ptr<Lexeme> LexerLine::getPreviousLexeme() {
  return getLexeme(lexemeIndex - 1);
}

shared_ptr<Lexeme> LexerLine::getLastLexeme() {
  if (lexemes.empty()) return nullptr;
  return getLexeme(lexemes.size() - 1);
}

shared_ptr<Lexeme> LexerLine::getLexeme(int i) {
  if (i >= 0 && i < static_cast<int>(lexemes.size())) {
    lexemeIndex = i;
    return lexemes[i];
  }
  return nullptr;
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
  if (!lexemeStack.empty()) lexemeStack.pop();
}

string LexerLine::toString() {
  string out = lineText;
  if (!out.empty())
    if (out.back() != '\n' && out.back() != '\r') out += "\n";
  for (auto& lexeme : lexemes) {
    out += lexeme->toString();
  }
  return out;
}

bool LexerLine::evaluate() {
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
