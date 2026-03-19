#ifndef COMPILER_START_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_START_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerStartStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_start(shared_ptr<CompilerContext> context);

 public:
  //! @brief execute statement code
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_START_STATEMENT_STRATEGY_H_INCLUDED
