#ifndef SQR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define SQR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class SqrCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(shared_ptr<CompilerContext> context,
              shared_ptr<ActionNode> action, int* result,
              unsigned int parmCount) override;
};

#endif  // SQR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
