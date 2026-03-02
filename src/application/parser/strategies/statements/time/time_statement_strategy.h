#ifndef TIME_STATEMENT_STRATEGY_H_INCLUDED
#define TIME_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class TimeStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // TIME_STATEMENT_STRATEGY_H_INCLUDED
