#ifndef COMPILER_LET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_LET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerLetStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_LET_STATEMENT_STRATEGY_H_INCLUDED
