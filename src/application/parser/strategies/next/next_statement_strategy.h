#ifndef NEXT_STATEMENT_STRATEGY_H_INCLUDED
#define NEXT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class NextStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // NEXT_STATEMENT_STRATEGY_H_INCLUDED
