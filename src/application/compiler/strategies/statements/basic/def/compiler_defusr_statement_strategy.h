#ifndef COMPILER_DEFUSR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFUSR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefUsrStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_defusr(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_DEFUSR_STATEMENT_STRATEGY_H_INCLUDED
