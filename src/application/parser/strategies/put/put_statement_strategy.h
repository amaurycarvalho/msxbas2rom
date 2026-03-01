#ifndef PUT_STATEMENT_STRATEGY_H_INCLUDED
#define PUT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PutStatementStrategy : public IParserStatementStrategy {
 public:
  bool parsePutSprite(Parser& parser, LexerLine* statement);
  bool parsePutTile(Parser& parser, LexerLine* statement);
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // PUT_STATEMENT_STRATEGY_H_INCLUDED
