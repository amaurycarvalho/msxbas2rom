#ifndef PUT_STATEMENT_STRATEGY_H_INCLUDED
#define PUT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PutStatementStrategy : public IParserStatementStrategy {
 public:
  bool parsePutSprite(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool parsePutTile(shared_ptr<ParserContext> context,
                    shared_ptr<LexerLineContext> statement);
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // PUT_STATEMENT_STRATEGY_H_INCLUDED
