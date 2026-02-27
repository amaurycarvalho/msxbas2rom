#ifndef LEXER_LINE_STATE_H_INCLUDED
#define LEXER_LINE_STATE_H_INCLUDED

#include "lexer_line.h"

enum class LexerLineProcessResult {
  Continue,
  Accept,
  Reject,
};

class LexerLineStateContext {
 private:
  bool isNumeric(char c) const;

 public:
  LexerLine* lexerLine;
  int index;
  int length;
  bool hexa;
  char current;
  Lexeme* lexeme;

  explicit LexerLineStateContext(LexerLine* lexerLine);

  bool hasNextChar() const;
  char peekNextChar() const;
  string restFromNext() const;

  bool isDecimal(char c) const;
  bool isHexDecimal(char c) const;
  bool isSeparator(char c) const;
  bool isOperator(char c) const;
  bool isIdentifier(char c, bool start) const;
  bool isComment(char c) const;

  void pushCurrentLexeme();
  void pushCurrentLexemeAndReset();
  void rewindCurrentChar();
  void normalizeKeyword();
};

class ILexerLineState {
 public:
  virtual ~ILexerLineState() = default;
  virtual LexerLineProcessResult handle(LexerLineStateContext& context) = 0;
};

#endif  // LEXER_LINE_STATE_H_INCLUDED
