#ifndef SET_STATEMENT_STRATEGY_H_INCLUDED
#define SET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSetAdjust(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool parseSetTile(shared_ptr<ParserContext> context,
                    shared_ptr<LexerLineContext> statement);
  bool parseSetTileColpat(shared_ptr<ParserContext> context,
                          shared_ptr<LexerLineContext> statement);
  bool parseSetSprite(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool parseSetSpriteColpattra(shared_ptr<ParserContext> context,
                               shared_ptr<LexerLineContext> statement);
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // SET_STATEMENT_STRATEGY_H_INCLUDED
