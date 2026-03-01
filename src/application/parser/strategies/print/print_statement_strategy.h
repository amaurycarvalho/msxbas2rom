#ifndef PRINT_STATEMENT_STRATEGY_H_INCLUDED
#define PRINT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PrintStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // PRINT_STATEMENT_STRATEGY_H_INCLUDED
