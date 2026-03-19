#ifndef INPUT_STRING_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define INPUT_STRING_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class InputStringCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // INPUT_STRING_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
