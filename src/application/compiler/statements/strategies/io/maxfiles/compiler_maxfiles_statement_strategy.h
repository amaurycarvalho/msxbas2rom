#ifndef COMPILER_MAXFILES_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_MAXFILES_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerMaxfilesStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_maxfiles(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_MAXFILES_STATEMENT_STRATEGY_H_INCLUDED
