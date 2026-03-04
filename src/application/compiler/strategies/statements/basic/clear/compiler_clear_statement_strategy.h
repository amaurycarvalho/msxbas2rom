#ifndef COMPILER_CLEAR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CLEAR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerClearStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_CLEAR_STATEMENT_STRATEGY_H_INCLUDED
