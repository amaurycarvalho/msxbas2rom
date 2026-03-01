#ifndef GENERIC_STATEMENT_STRATEGY_H_INCLUDED
#define GENERIC_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GenericStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // GENERIC_STATEMENT_STRATEGY_H_INCLUDED
