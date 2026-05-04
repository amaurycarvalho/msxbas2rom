#ifndef COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPrintStatementStrategy : public ICompilerStatementStrategy {
 private:
  bool isFilePrint(shared_ptr<CompilerContext> context);
  void cmd_file_print(shared_ptr<CompilerContext> context);
  void cmd_normal_print(shared_ptr<CompilerContext> context);
  void cmd_print(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_PRINT_STATEMENT_STRATEGY_H_INCLUDED
