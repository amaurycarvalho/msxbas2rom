#ifndef SCREEN_STATEMENT_STRATEGY_H_INCLUDED
#define SCREEN_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ScreenStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseScreenCopy(ParserContext& context, LexerLineContext* statement);
  bool parseScreenPaste(ParserContext& context, LexerLineContext* statement);
  bool parseScreenScroll(ParserContext& context, LexerLineContext* statement);
  bool parseScreenLoad(ParserContext& context, LexerLineContext* statement);
  bool parseScreenOn(ParserContext& context, LexerLineContext* statement);
  bool parseScreenOff(ParserContext& context, LexerLineContext* statement);
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // SCREEN_STATEMENT_STRATEGY_H_INCLUDED
