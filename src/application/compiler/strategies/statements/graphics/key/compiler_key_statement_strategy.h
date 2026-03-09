#ifndef COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerKeyStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_key(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_KEY_STATEMENT_STRATEGY_H_INCLUDED
