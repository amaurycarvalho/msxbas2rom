#ifndef COMPILER_DEFDBL_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFDBL_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefdblStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_DEFDBL_STATEMENT_STRATEGY_H_INCLUDED
