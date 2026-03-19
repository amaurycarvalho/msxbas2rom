#ifndef COMPILER_IPOKE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IPOKE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIpokeStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_ipoke(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_IPOKE_STATEMENT_STRATEGY_H_INCLUDED
