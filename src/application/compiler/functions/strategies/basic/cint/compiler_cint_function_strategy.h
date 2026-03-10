#ifndef CINT_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define CINT_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class CintCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // CINT_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
