#ifndef COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPrintStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED
