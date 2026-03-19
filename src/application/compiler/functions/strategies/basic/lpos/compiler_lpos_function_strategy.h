#ifndef LPOS_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define LPOS_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class LposCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // LPOS_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
