#ifndef COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGetStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED
