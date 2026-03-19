#ifndef LEXER_LINE_STATE_H_INCLUDED
#define LEXER_LINE_STATE_H_INCLUDED

#include <memory>
#include <string>

class Lexeme;
class LexerLineContext;
class LexerLineEvaluator;

enum class LexerLineProcessResult {
  Continue,
  Accept,
  Reject,
};

using namespace std;

class LexerLineStateContext {
 private:
  bool isNumeric(char c) const;

 public:
  LexerLineContext* lexerLine;
  int index;
  int length;
  bool hexa;
  char current;
  shared_ptr<Lexeme> lexeme;

  explicit LexerLineStateContext(LexerLineContext* lexerLine);
  ~LexerLineStateContext();

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
