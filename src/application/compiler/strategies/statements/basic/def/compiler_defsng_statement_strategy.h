#ifndef COMPILER_DEFSNG_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFSNG_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefsngStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_DEFSNG_STATEMENT_STRATEGY_H_INCLUDED
