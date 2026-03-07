#ifndef COMPILER_NEXT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_NEXT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerNextStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_next(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_NEXT_STATEMENT_STRATEGY_H_INCLUDED
