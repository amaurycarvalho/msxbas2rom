#ifndef COMPILER_IREAD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IREAD_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIreadStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_IREAD_STATEMENT_STRATEGY_H_INCLUDED
