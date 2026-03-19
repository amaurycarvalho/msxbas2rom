#ifndef COMPILER_DEFDBL_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFDBL_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefdblStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_DEFDBL_STATEMENT_STRATEGY_H_INCLUDED
