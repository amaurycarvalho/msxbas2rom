#ifndef DIM_STATEMENT_STRATEGY_H_INCLUDED
#define DIM_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DimStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // DIM_STATEMENT_STRATEGY_H_INCLUDED
