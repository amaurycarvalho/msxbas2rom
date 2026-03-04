#ifndef COMPILER_PAINT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PAINT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPaintStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_PAINT_STATEMENT_STRATEGY_H_INCLUDED
