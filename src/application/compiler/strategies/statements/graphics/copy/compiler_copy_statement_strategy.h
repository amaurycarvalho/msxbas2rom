#ifndef COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCopyStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_COPY_STATEMENT_STRATEGY_H_INCLUDED
