#ifndef COMPILER_COLOR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_COLOR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerColorStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_COLOR_STATEMENT_STRATEGY_H_INCLUDED
