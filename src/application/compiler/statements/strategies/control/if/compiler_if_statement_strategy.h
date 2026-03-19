#ifndef COMPILER_IF_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IF_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIfStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_if(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_IF_STATEMENT_STRATEGY_H_INCLUDED
