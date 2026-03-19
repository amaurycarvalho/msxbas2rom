#ifndef COMPILER_END_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_END_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerEndStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_end(shared_ptr<CompilerContext> context, bool doCodeRegistering);

 public:
  //! @brief execute statement code
  bool execute(shared_ptr<CompilerContext> context) override;
  //! @brief register statement code
  bool registerEndRoutine(shared_ptr<CompilerContext> context);
};

#endif  // COMPILER_END_STATEMENT_STRATEGY_H_INCLUDED
