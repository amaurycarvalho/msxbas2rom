#ifndef COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCopyStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_copy(shared_ptr<CompilerContext> context);
  void cmd_copy_screen(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED
