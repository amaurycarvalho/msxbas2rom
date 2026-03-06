#ifndef COMPILER_STRIG_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STRIG_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerStrigStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_STRIG_STATEMENT_STRATEGY_H_INCLUDED
