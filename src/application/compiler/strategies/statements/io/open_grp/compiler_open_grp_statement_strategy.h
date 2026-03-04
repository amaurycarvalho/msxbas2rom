#ifndef COMPILER_OPEN_GRP_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_OPEN_GRP_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerOpenGrpStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_OPEN_GRP_STATEMENT_STRATEGY_H_INCLUDED
