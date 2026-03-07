#ifndef COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGosubStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_gosub(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED
