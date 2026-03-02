#ifndef NOOP_STATEMENT_STRATEGY_H_INCLUDED
#define NOOP_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class NoopStatementStrategy : public IParserStatementStrategy {
 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // NOOP_STATEMENT_STRATEGY_H_INCLUDED
