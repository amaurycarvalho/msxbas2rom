#ifndef COMPILER_CALL_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CALL_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCallStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_CALL_STATEMENT_STRATEGY_H_INCLUDED
