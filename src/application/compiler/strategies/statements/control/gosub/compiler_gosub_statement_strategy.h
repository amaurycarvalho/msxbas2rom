#ifndef COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGosubStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_GOSUB_STATEMENT_STRATEGY_H_INCLUDED
