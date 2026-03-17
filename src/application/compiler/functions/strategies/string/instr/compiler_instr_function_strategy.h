#ifndef INSTR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define INSTR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include "compiler_function_strategy.h"

class ActionNode;
class CompilerContext;

class InstrCompilerFunctionStrategy : public ICompilerFunctionStrategy {
 public:
  int execute(CompilerContext* context, shared_ptr<ActionNode> action,
              int* result, unsigned int parmCount) override;
};

#endif  // INSTR_COMPILER_FUNCTION_STRATEGY_H_INCLUDED
