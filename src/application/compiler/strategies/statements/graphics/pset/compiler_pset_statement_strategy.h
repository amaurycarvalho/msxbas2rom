#ifndef COMPILER_PSET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PSET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPsetStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PSET_STATEMENT_STRATEGY_H_INCLUDED
