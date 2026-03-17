#ifndef BIN_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define BIN_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class BinCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context, shared_ptr<ActionNode> action,
              int* result, unsigned int parmCount) override;
};

#endif  // BIN_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
