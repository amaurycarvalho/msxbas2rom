#ifndef IDATA_STATEMENT_STRATEGY_H_INCLUDED
#define IDATA_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class IDataStatementStrategy : public IParserStatementStrategy {
 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // IDATA_STATEMENT_STRATEGY_H_INCLUDED
