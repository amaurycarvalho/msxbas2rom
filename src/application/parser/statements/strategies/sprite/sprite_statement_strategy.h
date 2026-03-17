#ifndef SPRITE_STATEMENT_STRATEGY_H_INCLUDED
#define SPRITE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SpriteStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSpriteLoad(ParserContext& context, LexerLine* statement);
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // SPRITE_STATEMENT_STRATEGY_H_INCLUDED
