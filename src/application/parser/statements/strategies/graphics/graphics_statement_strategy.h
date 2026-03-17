#ifndef GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
#define GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GraphicsStatementStrategy : public IParserStatementStrategy {
 private:
  bool parsePset(ParserContext& context, LexerLine* statement);
  bool parseLine(ParserContext& context, LexerLine* statement);
  bool parseCircle(ParserContext& context, LexerLine* statement);
  bool parsePaint(ParserContext& context, LexerLine* statement);
  bool parseCopy(ParserContext& context, LexerLine* statement);

 public:
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
