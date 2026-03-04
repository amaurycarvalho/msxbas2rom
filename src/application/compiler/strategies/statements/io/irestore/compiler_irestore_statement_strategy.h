#ifndef COMPILER_IRESTORE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IRESTORE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIrestoreStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_IRESTORE_STATEMENT_STRATEGY_H_INCLUDED
