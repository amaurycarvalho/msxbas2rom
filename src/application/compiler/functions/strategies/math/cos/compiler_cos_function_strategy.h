#ifndef COS_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define COS_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class CosCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // COS_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
