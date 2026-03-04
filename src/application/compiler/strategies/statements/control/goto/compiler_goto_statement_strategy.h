#ifndef COMPILER_GOTO_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GOTO_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGotoStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_GOTO_STATEMENT_STRATEGY_H_INCLUDED
