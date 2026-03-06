#ifndef COMPILER_END_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_END_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerEndStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_END_STATEMENT_STRATEGY_H_INCLUDED
