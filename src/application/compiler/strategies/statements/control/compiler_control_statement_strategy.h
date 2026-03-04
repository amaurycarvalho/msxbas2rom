#ifndef CONTROL_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define CONTROL_COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class ControlCompilerStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // CONTROL_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
