#ifndef COMPILER_DEFSTR_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DEFSTR_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDefstrStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_DEFSTR_STATEMENT_STRATEGY_H_INCLUDED
