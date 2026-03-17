#ifndef CMD_STATEMENT_STRATEGY_H_INCLUDED
#define CMD_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class CmdStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // CMD_STATEMENT_STRATEGY_H_INCLUDED
