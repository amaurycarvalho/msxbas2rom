#ifndef COMPILER_CALL_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CALL_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCallStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_call(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_CALL_STATEMENT_STRATEGY_H_INCLUDED
