#ifndef NOOP_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define NOOP_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class NoopCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // NOOP_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
