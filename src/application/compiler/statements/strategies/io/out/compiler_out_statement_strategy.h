#ifndef COMPILER_OUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_OUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerOutStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_out(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_OUT_STATEMENT_STRATEGY_H_INCLUDED
