#ifndef COMPILER_LINE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_LINE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerLineStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_line(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_LINE_STATEMENT_STRATEGY_H_INCLUDED
