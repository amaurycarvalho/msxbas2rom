#ifndef COLOR_STATEMENT_STRATEGY_H_INCLUDED
#define COLOR_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ColorStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseColorRgb(ParserContext& context, LexerLineContext* statement);
  bool parseColorSprite(ParserContext& context, LexerLineContext* statement);

 public:
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // COLOR_STATEMENT_STRATEGY_H_INCLUDED
