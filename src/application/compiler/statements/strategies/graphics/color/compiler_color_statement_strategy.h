#ifndef COMPILER_COLOR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_COLOR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerColorStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_color(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_COLOR_STATEMENT_STRATEGY_H_INCLUDED
