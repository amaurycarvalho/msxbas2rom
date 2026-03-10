#ifndef ASC_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define ASC_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class AscCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // ASC_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
