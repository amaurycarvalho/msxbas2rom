#ifndef COMPILER_BLOAD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_BLOAD_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerBloadStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_bload(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_BLOAD_STATEMENT_STRATEGY_H_INCLUDED
