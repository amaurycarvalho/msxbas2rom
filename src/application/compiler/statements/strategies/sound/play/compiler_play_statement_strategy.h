#ifndef COMPILER_PLAY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PLAY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPlayStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_play(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_PLAY_STATEMENT_STRATEGY_H_INCLUDED
