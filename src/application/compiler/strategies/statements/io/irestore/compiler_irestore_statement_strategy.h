#ifndef COMPILER_IRESTORE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IRESTORE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIrestoreStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_IRESTORE_STATEMENT_STRATEGY_H_INCLUDED
