#ifndef COMPILER_BEEP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_BEEP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerBeepStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_beep(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_BEEP_STATEMENT_STRATEGY_H_INCLUDED
