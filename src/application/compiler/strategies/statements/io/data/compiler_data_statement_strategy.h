#ifndef COMPILER_DATA_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DATA_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDataStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_DATA_STATEMENT_STRATEGY_H_INCLUDED
