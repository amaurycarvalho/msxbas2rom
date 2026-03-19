#ifndef COMPILER_FOR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_FOR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerForStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_for(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_FOR_STATEMENT_STRATEGY_H_INCLUDED
