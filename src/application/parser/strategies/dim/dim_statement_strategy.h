#ifndef DIM_STATEMENT_STRATEGY_H_INCLUDED
#define DIM_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DimStatementStrategy : public IParserStatementStrategy {
 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // DIM_STATEMENT_STRATEGY_H_INCLUDED
