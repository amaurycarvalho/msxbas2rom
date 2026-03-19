#ifndef STRING_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define STRING_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class StringCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // STRING_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
