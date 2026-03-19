#ifndef COMPILER_DIM_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DIM_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDimStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_dim(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_DIM_STATEMENT_STRATEGY_H_INCLUDED
