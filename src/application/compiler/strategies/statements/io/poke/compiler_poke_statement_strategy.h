#ifndef COMPILER_POKE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_POKE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPokeStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_POKE_STATEMENT_STRATEGY_H_INCLUDED
