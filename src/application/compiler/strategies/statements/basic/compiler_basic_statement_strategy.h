#ifndef BASIC_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define BASIC_COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class BasicCompilerStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // BASIC_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
