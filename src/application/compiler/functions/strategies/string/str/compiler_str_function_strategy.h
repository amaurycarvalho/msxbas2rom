#ifndef STR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define STR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class StrCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // STR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
