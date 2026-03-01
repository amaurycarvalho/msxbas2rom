#ifndef GET_STATEMENT_STRATEGY_H_INCLUDED
#define GET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseGetTile(Parser& parser, LexerLine* statement);
  bool parseGetSprite(Parser& parser, LexerLine* statement);
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // GET_STATEMENT_STRATEGY_H_INCLUDED
