#ifndef RESOURCESIZE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define RESOURCESIZE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class ResourcesizeCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context, shared_ptr<ActionNode> action,
              int* result, unsigned int parmCount) override;
};

#endif  // RESOURCESIZE_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
