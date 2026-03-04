#ifndef COMPILER_DEF_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEF_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_DEF_STATEMENT_STRATEGY_H_INCLUDED
