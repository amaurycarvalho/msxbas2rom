#ifndef COMPILER_CLS_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CLS_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerClsStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_CLS_STATEMENT_STRATEGY_H_INCLUDED
