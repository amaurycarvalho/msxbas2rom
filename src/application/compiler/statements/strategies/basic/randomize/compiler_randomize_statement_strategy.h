#ifndef COMPILER_RANDOMIZE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RANDOMIZE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerRandomizeStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_randomize(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_RANDOMIZE_STATEMENT_STRATEGY_H_INCLUDED
