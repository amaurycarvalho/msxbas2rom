/***
 * @file compiler_function_strategy_factory.cpp
 * @brief Compiler function strategy factory
 */

#include "compiler_function_strategy_factory.h"

CompilerFunctionStrategyFactory::CompilerFunctionStrategyFactory() {
  strategies["REM"] = &noopStrategy;
  strategies["'"] = &noopStrategy;
}

ICompilerFunctionStrategy*
CompilerFunctionStrategyFactory::getStrategyByKeyword(const string& keyword) {
  map<string, ICompilerFunctionStrategy*>::iterator it =
      strategies.find(keyword);
  if (it == strategies.end()) return 0;

  return it->second;
}
