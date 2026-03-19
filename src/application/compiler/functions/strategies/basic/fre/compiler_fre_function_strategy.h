#ifndef FRE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define FRE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class FreCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // FRE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
