#ifndef COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define COMPILER_FUNCTION_STRATEGY_H_INCLUDED

class CompilerContext;
class ActionNode;

class ICompilerFunctionStrategy {
 public:
  virtual ~ICompilerFunctionStrategy() {}
  virtual int execute(CompilerContext* context,
                      ActionNode* action,
                      int* result,
                      unsigned int parmCount) = 0;
};

#endif  // COMPILER_FUNCTION_STRATEGY_H_INCLUDED
