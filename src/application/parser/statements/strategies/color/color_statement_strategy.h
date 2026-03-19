#ifndef COLOR_STATEMENT_STRATEGY_H_INCLUDED
#define COLOR_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ColorStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseColorRgb(shared_ptr<ParserContext> context,
                     shared_ptr<LexerLineContext> statement);
  bool parseColorSprite(shared_ptr<ParserContext> context,
                        shared_ptr<LexerLineContext> statement);

 public:
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // COLOR_STATEMENT_STRATEGY_H_INCLUDED
