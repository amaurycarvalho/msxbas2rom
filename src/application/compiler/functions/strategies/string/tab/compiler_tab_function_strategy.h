#ifndef TAB_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define TAB_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class TabCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // TAB_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
