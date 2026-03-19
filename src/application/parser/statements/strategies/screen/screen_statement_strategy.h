#ifndef SCREEN_STATEMENT_STRATEGY_H_INCLUDED
#define SCREEN_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ScreenStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseScreenCopy(shared_ptr<ParserContext> context,
                       shared_ptr<LexerLineContext> statement);
  bool parseScreenPaste(shared_ptr<ParserContext> context,
                        shared_ptr<LexerLineContext> statement);
  bool parseScreenScroll(shared_ptr<ParserContext> context,
                         shared_ptr<LexerLineContext> statement);
  bool parseScreenLoad(shared_ptr<ParserContext> context,
                       shared_ptr<LexerLineContext> statement);
  bool parseScreenOn(shared_ptr<ParserContext> context,
                     shared_ptr<LexerLineContext> statement);
  bool parseScreenOff(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // SCREEN_STATEMENT_STRATEGY_H_INCLUDED
