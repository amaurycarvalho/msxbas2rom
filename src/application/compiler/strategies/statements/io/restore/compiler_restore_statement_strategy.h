#ifndef COMPILER_RESTORE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RESTORE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerRestoreStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_RESTORE_STATEMENT_STRATEGY_H_INCLUDED
