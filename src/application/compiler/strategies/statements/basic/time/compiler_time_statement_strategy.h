#ifndef COMPILER_TIME_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_TIME_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerTimeStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_TIME_STATEMENT_STRATEGY_H_INCLUDED
