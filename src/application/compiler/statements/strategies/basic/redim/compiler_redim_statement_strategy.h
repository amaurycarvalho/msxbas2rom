#ifndef COMPILER_REDIM_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_REDIM_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerRedimStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_REDIM_STATEMENT_STRATEGY_H_INCLUDED
