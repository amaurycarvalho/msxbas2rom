#ifndef MAKER_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define MAKER_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class MakerCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context, shared_ptr<ActionNode> action,
              int* result, unsigned int parmCount) override;
};

#endif  // MAKER_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
