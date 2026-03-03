#ifndef GET_STATEMENT_STRATEGY_H_INCLUDED
#define GET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseGetTile(ParserContext& context, LexerLine* statement);
  bool parseGetSprite(ParserContext& context, LexerLine* statement);
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // GET_STATEMENT_STRATEGY_H_INCLUDED
