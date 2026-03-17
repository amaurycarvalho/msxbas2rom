#ifndef SPRITE_STATEMENT_STRATEGY_H_INCLUDED
#define SPRITE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SpriteStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSpriteLoad(ParserContext& context, LexerLineContext* statement);
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // SPRITE_STATEMENT_STRATEGY_H_INCLUDED
