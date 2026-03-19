#ifndef COMPILER_INTERVAL_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_INTERVAL_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIntervalStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_interval(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_INTERVAL_STATEMENT_STRATEGY_H_INCLUDED
