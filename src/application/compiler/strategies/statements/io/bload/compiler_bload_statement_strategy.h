#ifndef COMPILER_BLOAD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_BLOAD_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerBloadStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_BLOAD_STATEMENT_STRATEGY_H_INCLUDED
