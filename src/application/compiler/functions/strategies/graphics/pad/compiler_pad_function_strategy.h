#ifndef PAD_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define PAD_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class PadCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // PAD_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
