#ifndef COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerKeyStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_key(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED
