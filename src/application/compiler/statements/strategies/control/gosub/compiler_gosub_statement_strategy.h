#ifndef COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGosubStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_gosub(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED
