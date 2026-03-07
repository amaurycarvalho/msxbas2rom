#ifndef COMPILER_GOTO_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GOTO_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGotoStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_goto(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_GOTO_STATEMENT_STRATEGY_H_INCLUDED
