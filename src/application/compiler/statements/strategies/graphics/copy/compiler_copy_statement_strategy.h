#ifndef COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCopyStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_copy(CompilerContext* context);
  void cmd_copy_screen(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED
