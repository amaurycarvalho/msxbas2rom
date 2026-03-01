#ifndef SCREEN_STATEMENT_STRATEGY_H_INCLUDED
#define SCREEN_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ScreenStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseScreenCopy(Parser& parser, LexerLine* statement);
  bool parseScreenPaste(Parser& parser, LexerLine* statement);
  bool parseScreenScroll(Parser& parser, LexerLine* statement);
  bool parseScreenLoad(Parser& parser, LexerLine* statement);
  bool parseScreenOn(Parser& parser, LexerLine* statement);
  bool parseScreenOff(Parser& parser, LexerLine* statement);
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // SCREEN_STATEMENT_STRATEGY_H_INCLUDED
