#ifndef COMPILER_IDATA_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IDATA_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIdataStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_idata(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_IDATA_STATEMENT_STRATEGY_H_INCLUDED
