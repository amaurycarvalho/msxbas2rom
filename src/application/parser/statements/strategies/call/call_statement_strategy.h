#ifndef CALL_STATEMENT_STRATEGY_H_INCLUDED
#define CALL_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class CallStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseCall(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // CALL_STATEMENT_STRATEGY_H_INCLUDED
