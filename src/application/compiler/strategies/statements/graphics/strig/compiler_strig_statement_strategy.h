#ifndef COMPILER_STRIG_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STRIG_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerStrigStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_strig(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_STRIG_STATEMENT_STRATEGY_H_INCLUDED
