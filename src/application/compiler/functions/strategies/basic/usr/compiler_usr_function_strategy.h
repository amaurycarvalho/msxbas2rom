#ifndef USR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define USR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class UsrCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // USR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
