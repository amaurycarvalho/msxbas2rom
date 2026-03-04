#ifndef IO_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define IO_COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class IoCompilerStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // IO_COMPILER_STATEMENT_STRATEGY_H_INCLUDED
