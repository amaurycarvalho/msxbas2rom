#ifndef COMPILER_RETURN_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RETURN_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerReturnStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_RETURN_STATEMENT_STRATEGY_H_INCLUDED
