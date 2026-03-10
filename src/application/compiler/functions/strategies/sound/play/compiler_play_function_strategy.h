#ifndef PLAY_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define PLAY_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class PlayCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context,
              ActionNode* action,
              int* result,
              unsigned int parmCount) override;
};

#endif  // PLAY_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
