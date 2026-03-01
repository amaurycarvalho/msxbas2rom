#ifndef INPUT_STATEMENT_STRATEGY_H_INCLUDED
#define INPUT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class InputStatementStrategy : public IParserStatementStrategy {
 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // INPUT_STATEMENT_STRATEGY_H_INCLUDED
