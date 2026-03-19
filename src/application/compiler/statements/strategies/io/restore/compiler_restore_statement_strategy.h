#ifndef COMPILER_RESTORE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RESTORE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerRestoreStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_restore(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_RESTORE_STATEMENT_STRATEGY_H_INCLUDED
