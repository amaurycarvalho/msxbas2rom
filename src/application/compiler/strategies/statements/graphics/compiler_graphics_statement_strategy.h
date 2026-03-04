#ifndef GRAPHICS_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define GRAPHICS_COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class GraphicsCompilerStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // GRAPHICS_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
