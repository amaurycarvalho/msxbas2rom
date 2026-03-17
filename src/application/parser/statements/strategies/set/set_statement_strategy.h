#ifndef SET_STATEMENT_STRATEGY_H_INCLUDED
#define SET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSetAdjust(ParserContext& context, LexerLineContext* statement);
  bool parseSetTile(ParserContext& context, LexerLineContext* statement);
  bool parseSetTileColpat(ParserContext& context, LexerLineContext* statement);
  bool parseSetSprite(ParserContext& context, LexerLineContext* statement);
  bool parseSetSpriteColpattra(ParserContext& context,
                               LexerLineContext* statement);
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // SET_STATEMENT_STRATEGY_H_INCLUDED
