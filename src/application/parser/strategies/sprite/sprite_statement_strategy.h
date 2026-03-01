#ifndef SPRITE_STATEMENT_STRATEGY_H_INCLUDED
#define SPRITE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class SpriteStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseSpriteLoad(Parser& parser, LexerLine* statement);
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // SPRITE_STATEMENT_STRATEGY_H_INCLUDED
