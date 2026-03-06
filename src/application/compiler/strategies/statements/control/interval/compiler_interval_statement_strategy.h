#ifndef COMPILER_INTERVAL_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_INTERVAL_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIntervalStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_INTERVAL_STATEMENT_STRATEGY_H_INCLUDED
