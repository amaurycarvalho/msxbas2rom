#ifndef COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_context.h"

class ICompilerStatementStrategy {
 public:
  virtual ~ICompilerStatementStrategy() {}
  virtual bool execute(CompilerStatementContext& ctx) = 0;
};

#endif  // COMPILER_STATEMENT_STRATEGY_H_INCLUDED
