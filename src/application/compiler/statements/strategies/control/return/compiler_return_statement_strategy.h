#ifndef COMPILER_RETURN_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RETURN_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerReturnStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_return(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_RETURN_STATEMENT_STRATEGY_H_INCLUDED
