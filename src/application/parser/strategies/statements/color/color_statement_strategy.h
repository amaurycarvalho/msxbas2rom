#ifndef COLOR_STATEMENT_STRATEGY_H_INCLUDED
#define COLOR_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ColorStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseColorRgb(ParserContext& context, LexerLine* statement);
  bool parseColorSprite(ParserContext& context, LexerLine* statement);

 public:
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // COLOR_STATEMENT_STRATEGY_H_INCLUDED
