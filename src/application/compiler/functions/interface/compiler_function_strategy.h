#ifndef COMPILER_FUNCTION_STRATEGY_H_INCLUDED
#define COMPILER_FUNCTION_STRATEGY_H_INCLUDED

class CompilerContext;

class ICompilerFunctionStrategy {
 public:
  virtual ~ICompilerFunctionStrategy() {}
  virtual bool execute(CompilerContext* context) = 0;
};

#endif  // COMPILER_FUNCTION_STRATEGY_H_INCLUDED
