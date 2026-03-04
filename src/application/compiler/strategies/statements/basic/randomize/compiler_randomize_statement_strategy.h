#ifndef COMPILER_RANDOMIZE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RANDOMIZE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerRandomizeStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_RANDOMIZE_STATEMENT_STRATEGY_H_INCLUDED
