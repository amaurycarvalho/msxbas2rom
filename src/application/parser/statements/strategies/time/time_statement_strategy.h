#ifndef TIME_STATEMENT_STRATEGY_H_INCLUDED
#define TIME_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class TimeStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // TIME_STATEMENT_STRATEGY_H_INCLUDED
