#ifndef DEF_STATEMENT_STRATEGY_H_INCLUDED
#define DEF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DefStatementStrategy : public IParserStatementStrategy {
 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // DEF_STATEMENT_STRATEGY_H_INCLUDED
