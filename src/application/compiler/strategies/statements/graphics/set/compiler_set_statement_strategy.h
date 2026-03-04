#ifndef COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSetStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED
