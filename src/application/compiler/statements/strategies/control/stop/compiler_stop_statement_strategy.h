#ifndef COMPILER_STOP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STOP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerStopStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_stop(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_STOP_STATEMENT_STRATEGY_H_INCLUDED
