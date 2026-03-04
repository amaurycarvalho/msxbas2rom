#ifndef COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerKeyStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED
