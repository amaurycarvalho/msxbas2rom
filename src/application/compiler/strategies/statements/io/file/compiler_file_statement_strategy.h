#ifndef COMPILER_FILE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_FILE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerFileStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_file(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_FILE_STATEMENT_STRATEGY_H_INCLUDED
