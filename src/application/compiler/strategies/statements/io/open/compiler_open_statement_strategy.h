#ifndef COMPILER_OPEN_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_OPEN_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerOpenStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_OPEN_STATEMENT_STRATEGY_H_INCLUDED
