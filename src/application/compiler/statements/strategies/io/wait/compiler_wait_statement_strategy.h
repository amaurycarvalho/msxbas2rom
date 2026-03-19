#ifndef COMPILER_WAIT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_WAIT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerWaitStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_wait(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_WAIT_STATEMENT_STRATEGY_H_INCLUDED
