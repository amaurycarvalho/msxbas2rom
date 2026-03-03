#ifndef SET_STATEMENT_STRATEGY_H_INCLUDED
#define SET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSetAdjust(ParserContext& context, LexerLine* statement);
  bool parseSetTile(ParserContext& context, LexerLine* statement);
  bool parseSetTileColpat(ParserContext& context, LexerLine* statement);
  bool parseSetSprite(ParserContext& context, LexerLine* statement);
  bool parseSetSpriteColpattra(ParserContext& context, LexerLine* statement);
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // SET_STATEMENT_STRATEGY_H_INCLUDED
