#ifndef COMPILER_LOCATE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_LOCATE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerLocateStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_locate(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_LOCATE_STATEMENT_STRATEGY_H_INCLUDED
