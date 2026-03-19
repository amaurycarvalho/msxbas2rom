#ifndef COMPILER_IREAD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IREAD_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIreadStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_iread(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_IREAD_STATEMENT_STRATEGY_H_INCLUDED
