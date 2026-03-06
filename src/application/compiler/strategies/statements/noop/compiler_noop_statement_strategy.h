#ifndef NOOP_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define NOOP_COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class NoopCompilerStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // NOOP_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
