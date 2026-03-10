#ifndef COMPILER_DATA_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DATA_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDataStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_data(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_DATA_STATEMENT_STRATEGY_H_INCLUDED
