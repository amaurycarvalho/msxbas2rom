#ifndef COMPILER_FILE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_FILE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerFileStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_file(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_FILE_STATEMENT_STRATEGY_H_INCLUDED
