#ifndef CALL_STATEMENT_STRATEGY_H_INCLUDED
#define CALL_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class CallStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseCall(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // CALL_STATEMENT_STRATEGY_H_INCLUDED
