#ifndef COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerInputStatementStrategy : public ICompilerStatementStrategy {
 private:
  //! @brief INPUT statement
  //! @param context Compiler context
  //! @param question Show question mark?
  void cmd_input(shared_ptr<CompilerContext> context, bool questionMark);

 public:
  //! @brief INPUT statement
  bool execute(shared_ptr<CompilerContext> context) override;
  //! @brief LINE INPUT statement
  bool executeLineInput(shared_ptr<CompilerContext> context);
};

#endif  // COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED
