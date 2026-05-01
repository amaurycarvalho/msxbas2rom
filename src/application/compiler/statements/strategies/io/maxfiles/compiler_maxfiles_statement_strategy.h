#ifndef COMPILER_MAXFILES_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_MAXFILES_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerMaxfilesStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_fmaxfiles(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_MAXFILES_STATEMENT_STRATEGY_H_INCLUDED
