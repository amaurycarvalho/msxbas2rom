#ifndef FOR_STATEMENT_STRATEGY_H_INCLUDED
#define FOR_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ForStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // FOR_STATEMENT_STRATEGY_H_INCLUDED
