#ifndef COMPILER_READ_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_READ_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerReadStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_read(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_READ_STATEMENT_STRATEGY_H_INCLUDED
