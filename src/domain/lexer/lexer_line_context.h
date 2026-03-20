/***
 * @file lexer_line_context.h
 * @brief MSX-BASIC lexer line context
 * @author Amaury Carvalho (2019-2026)
 */

#ifndef LEXER_LINE_CONTEXT_H_INCLUDED
#define LEXER_LINE_CONTEXT_H_INCLUDED

#include <memory>
#include <stack>
#include <string>
#include <vector>

class Lexeme;

using namespace std;

/***
 * @class LexerLineContext
 * @brief It represents a set of MSX-BASIC lexemes forming a line.
 */
class LexerLineContext {
 protected:
  int lexemeIndex;
  vector<shared_ptr<Lexeme>> lexemes;
  stack<int> lexemeStack;

 public:
  string lineText;
  int lineNumber;

  string toString();
  void clearLexemes();
  void addLexeme(shared_ptr<Lexeme> lexeme);
  shared_ptr<Lexeme> getFirstLexeme();
  shared_ptr<Lexeme> getCurrentLexeme();
  shared_ptr<Lexeme> getNextLexeme();
  shared_ptr<Lexeme> getPreviousLexeme();
  shared_ptr<Lexeme> getLastLexeme();
  shared_ptr<Lexeme> getLexeme(int i);
  void setLexemeBOF();
  int getLexemeCount();
  void pushLexeme();
  void popLexeme();
  void popLexemeDiscarding();

  LexerLineContext();
  ~LexerLineContext();
};

#endif  // LEXER_LINE_CONTEXT_H_INCLUDED
