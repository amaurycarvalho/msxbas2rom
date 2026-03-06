#ifndef COMPILER_PLAY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PLAY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPlayStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PLAY_STATEMENT_STRATEGY_H_INCLUDED
