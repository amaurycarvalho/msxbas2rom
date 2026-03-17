#ifndef IDATA_STATEMENT_STRATEGY_H_INCLUDED
#define IDATA_STATEMENT_STRATEGY_H_INCLUDED

#include "data_statement_strategy.h"

class IDataStatementStrategy : public DataStatementStrategy {
 public:
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // IDATA_STATEMENT_STRATEGY_H_INCLUDED
