#ifndef SET_STATEMENT_STRATEGY_H_INCLUDED
#define SET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSetAdjust(Parser& parser, LexerLine* statement);
  bool parseSetTile(Parser& parser, LexerLine* statement);
  bool parseSetTileColpat(Parser& parser, LexerLine* statement);
  bool parseSetSprite(Parser& parser, LexerLine* statement);
  bool parseSetSpriteColpattra(Parser& parser, LexerLine* statement);
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // SET_STATEMENT_STRATEGY_H_INCLUDED
