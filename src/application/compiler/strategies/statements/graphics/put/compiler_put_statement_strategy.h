#ifndef COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPutStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED
