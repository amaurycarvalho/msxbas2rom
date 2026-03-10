#ifndef COMPILER_DEF_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEF_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_def(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_DEF_STATEMENT_STRATEGY_H_INCLUDED
