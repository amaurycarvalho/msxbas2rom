#ifndef COMPILER_FUNCTION_STRATEGY_FACTORY_H_INCLUDED
#define COMPILER_FUNCTION_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "compiler_function_strategy.h"
#include "compiler_noop_function_strategy.h"

using namespace std;

class CompilerFunctionStrategyFactory {
 private:
  NoopCompilerFunctionStrategy noopStrategy;

  map<string, ICompilerFunctionStrategy*> strategies;

 public:
  CompilerFunctionStrategyFactory();
  ICompilerFunctionStrategy* getStrategyByKeyword(const string& keyword);
};

#endif  // COMPILER_FUNCTION_STRATEGY_FACTORY_H_INCLUDED
