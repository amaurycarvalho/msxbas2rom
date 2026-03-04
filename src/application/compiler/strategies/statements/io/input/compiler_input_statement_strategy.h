#ifndef COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerInputStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED
