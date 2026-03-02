#ifndef IF_STATEMENT_STRATEGY_H_INCLUDED
#define IF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class IfStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement, int level);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // IF_STATEMENT_STRATEGY_H_INCLUDED
