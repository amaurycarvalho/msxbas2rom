/***
 * @file lexer_line.h
 * @brief MSX BASIC lexer line class header
 * @author Amaury Carvalho (2019-2025)
 */

#ifndef LEXER_LINE_H_INCLUDED
#define LEXER_LINE_H_INCLUDED

#include <stack>
#include <string>
#include <vector>

#include "lexeme.h"

using namespace std;

/***
 * @class LexerLine
 * @brief It represents a set of MSX BASIC lexemes forming a line.
 */
class LexerLine {
 private:
  int lexemeIndex;
  vector<Lexeme*> lexemes;
  stack<int> lexemeStack;

 public:
  string line;

  bool evaluate();
  void print();
  void clearLexemes();
  void addLexeme(Lexeme* lexeme);
  Lexeme* getFirstLexeme();
  Lexeme* getCurrentLexeme();
  Lexeme* getNextLexeme();
  Lexeme* getPreviousLexeme();
  Lexeme* getLastLexeme();
  Lexeme* getLexeme(int i);
  void setLexemeBOF();
  int getLexemeCount();
  void pushLexeme();
  void popLexeme();
  void popLexemeDiscarding();

  LexerLine();
};

#endif  // LEXER_LINE_H_INCLUDED
