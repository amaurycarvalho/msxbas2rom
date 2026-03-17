#ifndef COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define COMPILER_FUNCTION_STRATEGY_H_INCLUDED

#include <memory>

class CompilerContext;
class ActionNode;

using namespace std;

class ICompilerFunctionStrategy {
 public:
  virtual ~ICompilerFunctionStrategy() {}
  virtual int execute(CompilerContext* context, shared_ptr<ActionNode> action,
                      int* result, unsigned int parmCount) = 0;
};

#endif  // COMPILER_FUNCTION_STRATEGY_H_INCLUDED
