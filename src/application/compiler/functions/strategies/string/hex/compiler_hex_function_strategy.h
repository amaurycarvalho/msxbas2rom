#ifndef HEX_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define HEX_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class HexCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context, shared_ptr<ActionNode> action,
              int* result, unsigned int parmCount) override;
};

#endif  // HEX_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
