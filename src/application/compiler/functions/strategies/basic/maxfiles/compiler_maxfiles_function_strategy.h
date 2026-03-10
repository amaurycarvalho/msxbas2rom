#ifndef MAXFILES_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define MAXFILES_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class MaxfilesCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // MAXFILES_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
