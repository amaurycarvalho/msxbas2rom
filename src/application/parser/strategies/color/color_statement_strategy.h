#ifndef COLOR_STATEMENT_STRATEGY_H_INCLUDED
#define COLOR_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ColorStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseColorRgb(Parser& parser, LexerLine* statement);
  bool parseColorSprite(Parser& parser, LexerLine* statement);

 public:
  bool parseStatement(Parser& parser, LexerLine* statement);
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // COLOR_STATEMENT_STRATEGY_H_INCLUDED
