#ifndef TIME_STATEMENT_STRATEGY_H_INCLUDED
#define TIME_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class TimeStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // TIME_STATEMENT_STRATEGY_H_INCLUDED
