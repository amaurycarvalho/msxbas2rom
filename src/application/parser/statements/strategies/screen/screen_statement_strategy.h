#ifndef SCREEN_STATEMENT_STRATEGY_H_INCLUDED
#define SCREEN_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ScreenStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseScreenCopy(ParserContext& context, LexerLine* statement);
  bool parseScreenPaste(ParserContext& context, LexerLine* statement);
  bool parseScreenScroll(ParserContext& context, LexerLine* statement);
  bool parseScreenLoad(ParserContext& context, LexerLine* statement);
  bool parseScreenOn(ParserContext& context, LexerLine* statement);
  bool parseScreenOff(ParserContext& context, LexerLine* statement);
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // SCREEN_STATEMENT_STRATEGY_H_INCLUDED
