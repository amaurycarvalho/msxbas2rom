#ifndef COMPILER_SWAP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SWAP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSwapStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_swap(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_SWAP_STATEMENT_STRATEGY_H_INCLUDED
