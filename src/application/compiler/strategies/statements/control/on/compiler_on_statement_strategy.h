#ifndef COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerOnStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED
