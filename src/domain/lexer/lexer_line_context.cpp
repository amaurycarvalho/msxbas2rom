/***
 * @file lexer_line_context.cpp
 * @brief MSX BASIC lexer line context
 * @author Amaury Carvalho (2019-2026)
 */

#include "lexer_line_context.h"

#include "lexeme.h"

/***
 * @name LexerLineContext class functions
 */

LexerLineContext::LexerLineContext() {
  lexemeIndex = 0;
}

LexerLineContext::~LexerLineContext() = default;

void LexerLineContext::clearLexemes() {
  lexemes.clear();
}

void LexerLineContext::addLexeme(shared_ptr<Lexeme> lexeme) {
  lexemes.emplace_back(lexeme);
}

void LexerLineContext::setLexemeBOF() {
  lexemeIndex = -1;
}

shared_ptr<Lexeme> LexerLineContext::getCurrentLexeme() {
  return getLexeme(lexemeIndex);
}

shared_ptr<Lexeme> LexerLineContext::getFirstLexeme() {
  return getLexeme(0);
}

shared_ptr<Lexeme> LexerLineContext::getNextLexeme() {
  return getLexeme(lexemeIndex + 1);
}

shared_ptr<Lexeme> LexerLineContext::getPreviousLexeme() {
  return getLexeme(lexemeIndex - 1);
}

shared_ptr<Lexeme> LexerLineContext::getLastLexeme() {
  if (lexemes.empty()) return nullptr;
  return getLexeme(lexemes.size() - 1);
}

shared_ptr<Lexeme> LexerLineContext::getLexeme(int i) {
  if (i >= 0 && i < static_cast<int>(lexemes.size())) {
    lexemeIndex = i;
    return lexemes[i];
  }
  return nullptr;
}

int LexerLineContext::getLexemeCount() {
  return lexemes.size();
}

void LexerLineContext::pushLexeme() {
  lexemeStack.push(lexemeIndex);
}

void LexerLineContext::popLexeme() {
  if (!lexemeStack.empty()) {
    lexemeIndex = lexemeStack.top();
    lexemeStack.pop();
  }
}

void LexerLineContext::popLexemeDiscarding() {
  if (!lexemeStack.empty()) lexemeStack.pop();
}

string LexerLineContext::toString() {
  string out = lineText;
  if (!out.empty())
    if (out.back() != '\n' && out.back() != '\r') out += "\n";
  for (auto& lexeme : lexemes) {
    out += lexeme->toString();
  }
  return out;
}
