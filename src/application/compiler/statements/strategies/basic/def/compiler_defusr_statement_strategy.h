#ifndef COMPILER_DEFUSR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFUSR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefUsrStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_defusr(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_DEFUSR_STATEMENT_STRATEGY_H_INCLUDED
