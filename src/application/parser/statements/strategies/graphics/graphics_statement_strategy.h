#ifndef GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
#define GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GraphicsStatementStrategy : public IParserStatementStrategy {
 private:
  bool parsePset(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement);
  bool parseLine(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement);
  bool parseCircle(shared_ptr<ParserContext> context,
                   shared_ptr<LexerLineContext> statement);
  bool parsePaint(shared_ptr<ParserContext> context,
                  shared_ptr<LexerLineContext> statement);
  bool parseCopy(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement);

 public:
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // GRAPHICS_STATEMENT_STRATEGY_H_INCLUDED
