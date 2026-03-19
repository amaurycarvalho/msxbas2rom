#ifndef COMPILER_READ_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_READ_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerReadStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_read(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_READ_STATEMENT_STRATEGY_H_INCLUDED
