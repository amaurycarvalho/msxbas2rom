#ifndef SPACE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define SPACE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class SpaceCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context, shared_ptr<ActionNode> action,
              int* result, unsigned int parmCount) override;
};

#endif  // SPACE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
