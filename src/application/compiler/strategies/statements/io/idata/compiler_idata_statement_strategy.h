#ifndef COMPILER_IDATA_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_IDATA_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerIdataStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_idata(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_IDATA_STATEMENT_STRATEGY_H_INCLUDED
