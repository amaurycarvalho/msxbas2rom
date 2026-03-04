#ifndef COMPILER_STOP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STOP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerStopStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_STOP_STATEMENT_STRATEGY_H_INCLUDED
