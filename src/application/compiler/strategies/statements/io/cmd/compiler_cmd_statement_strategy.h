#ifndef COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCmdStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED
