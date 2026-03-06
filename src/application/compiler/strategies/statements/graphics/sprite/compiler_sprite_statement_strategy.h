#ifndef COMPILER_SPRITE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SPRITE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSpriteStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_SPRITE_STATEMENT_STRATEGY_H_INCLUDED
