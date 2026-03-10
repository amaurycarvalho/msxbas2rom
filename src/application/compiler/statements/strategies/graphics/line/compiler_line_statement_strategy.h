#ifndef COMPILER_LINE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_LINE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerLineStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_line(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_LINE_STATEMENT_STRATEGY_H_INCLUDED
