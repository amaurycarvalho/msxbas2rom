#ifndef COMPILER_VPOKE_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_VPOKE_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerVpokeStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_vpoke(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_VPOKE_STATEMENT_STRATEGY_H_INCLUDED
