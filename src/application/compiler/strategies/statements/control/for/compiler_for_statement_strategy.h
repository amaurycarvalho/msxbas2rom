#ifndef COMPILER_FOR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_FOR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerForStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_for(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_FOR_STATEMENT_STRATEGY_H_INCLUDED
