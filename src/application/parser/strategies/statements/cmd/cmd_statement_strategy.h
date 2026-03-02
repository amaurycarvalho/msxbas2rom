#ifndef CMD_STATEMENT_STRATEGY_H_INCLUDED
#define CMD_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class CmdStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // CMD_STATEMENT_STRATEGY_H_INCLUDED
