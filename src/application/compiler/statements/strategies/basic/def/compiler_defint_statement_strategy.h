#ifndef COMPILER_DEFINT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFINT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefintStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_DEFINT_STATEMENT_STRATEGY_H_INCLUDED
