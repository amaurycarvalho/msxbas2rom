#ifndef STICK_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define STICK_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class StickCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // STICK_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
