#ifndef COMPILER_START_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_START_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerStartStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_start(CompilerContext* context);

 public:
  //! @brief execute statement code
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_START_STATEMENT_STRATEGY_H_INCLUDED
