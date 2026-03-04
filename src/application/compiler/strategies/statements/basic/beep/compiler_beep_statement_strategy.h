#ifndef COMPILER_BEEP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_BEEP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerBeepStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_BEEP_STATEMENT_STRATEGY_H_INCLUDED
