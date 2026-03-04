#ifndef COMPILER_DRAW_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DRAW_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDrawStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_DRAW_STATEMENT_STRATEGY_H_INCLUDED
