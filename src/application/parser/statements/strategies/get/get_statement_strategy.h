#ifndef GET_STATEMENT_STRATEGY_H_INCLUDED
#define GET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseGetTile(shared_ptr<ParserContext> context,
                    shared_ptr<LexerLineContext> statement);
  bool parseGetSprite(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // GET_STATEMENT_STRATEGY_H_INCLUDED
