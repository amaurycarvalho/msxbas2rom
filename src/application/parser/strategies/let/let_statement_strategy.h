#ifndef LET_STATEMENT_STRATEGY_H_INCLUDED
#define LET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class LetStatementStrategy : public IParserStatementStrategy {
 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // LET_STATEMENT_STRATEGY_H_INCLUDED
