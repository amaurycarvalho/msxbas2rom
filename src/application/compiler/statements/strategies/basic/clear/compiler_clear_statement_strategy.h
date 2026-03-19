#ifndef COMPILER_CLEAR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CLEAR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerClearStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_clear(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_CLEAR_STATEMENT_STRATEGY_H_INCLUDED
