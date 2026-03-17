#ifndef NEXT_STATEMENT_STRATEGY_H_INCLUDED
#define NEXT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class NextStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // NEXT_STATEMENT_STRATEGY_H_INCLUDED
