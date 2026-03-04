#ifndef COMPILER_IPOKE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IPOKE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIpokeStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_IPOKE_STATEMENT_STRATEGY_H_INCLUDED
