#ifndef GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
#define GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GraphicsStatementStrategy : public IParserStatementStrategy {
 private:
  bool parsePset(ParserContext& context, LexerLineContext* statement);
  bool parseLine(ParserContext& context, LexerLineContext* statement);
  bool parseCircle(ParserContext& context, LexerLineContext* statement);
  bool parsePaint(ParserContext& context, LexerLineContext* statement);
  bool parseCopy(ParserContext& context, LexerLineContext* statement);

 public:
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
