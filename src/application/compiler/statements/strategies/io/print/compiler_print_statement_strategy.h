#ifndef COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPrintStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_print(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED
