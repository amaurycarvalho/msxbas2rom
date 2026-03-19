#ifndef COMPILER_CLOSE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CLOSE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCloseStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_close(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_CLOSE_STATEMENT_STRATEGY_H_INCLUDED
