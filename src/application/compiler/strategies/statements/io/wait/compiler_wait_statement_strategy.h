#ifndef COMPILER_WAIT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_WAIT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerWaitStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_wait(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_WAIT_STATEMENT_STRATEGY_H_INCLUDED
