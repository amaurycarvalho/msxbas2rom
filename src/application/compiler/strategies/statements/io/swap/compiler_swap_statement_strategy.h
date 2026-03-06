#ifndef COMPILER_SWAP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SWAP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSwapStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_SWAP_STATEMENT_STRATEGY_H_INCLUDED
