#ifndef COMPILER_CIRCLE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CIRCLE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerCircleStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_CIRCLE_STATEMENT_STRATEGY_H_INCLUDED
