#ifndef COMPILER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define COMPILER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "compiler_statement_strategy.h"
#include "compiler_basic_statement_strategy.h"
#include "compiler_control_statement_strategy.h"
#include "compiler_graphics_statement_strategy.h"
#include "compiler_io_statement_strategy.h"
#include "compiler_noop_statement_strategy.h"

using namespace std;

class CompilerStatementStrategyFactory {
 private:
  NoopCompilerStatementStrategy noopStrategy;
  BasicCompilerStatementStrategy basicStrategy;
  ControlCompilerStatementStrategy controlStrategy;
  IoCompilerStatementStrategy ioStrategy;
  GraphicsCompilerStatementStrategy graphicsStrategy;
  map<string, ICompilerStatementStrategy*> strategies;

 public:
  CompilerStatementStrategyFactory();
  ICompilerStatementStrategy* getStrategyByKeyword(const string& keyword);
};

#endif  // COMPILER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
