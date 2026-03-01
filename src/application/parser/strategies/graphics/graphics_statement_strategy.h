#ifndef GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
#define GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GraphicsStatementStrategy : public IParserStatementStrategy {
 private:
  bool parsePset(Parser& parser, LexerLine* statement);
  bool parseLine(Parser& parser, LexerLine* statement);
  bool parseCircle(Parser& parser, LexerLine* statement);
  bool parsePaint(Parser& parser, LexerLine* statement);
  bool parseCopy(Parser& parser, LexerLine* statement);

 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
