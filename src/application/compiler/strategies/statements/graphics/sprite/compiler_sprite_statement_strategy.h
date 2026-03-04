#ifndef COMPILER_SPRITE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SPRITE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSpriteStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_SPRITE_STATEMENT_STRATEGY_H_INCLUDED
