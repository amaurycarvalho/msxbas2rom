#ifndef TILE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define TILE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class TileCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // TILE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
