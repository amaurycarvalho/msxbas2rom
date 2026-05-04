#ifndef COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerInputStatementStrategy : public ICompilerStatementStrategy {
 private:
  //! @brief Detect INPUT# statement
  bool isFileInput(shared_ptr<CompilerContext> context);

  //! @brief INPUT# statement
  //! @param lineMode false=INPUT# field mode, true=LINE INPUT# line mode
  void cmd_file_input(shared_ptr<CompilerContext> context, bool lineMode);

  //! @brief INPUT / LINE INPUT statement
  //! @param context Compiler context
  //! @param question Show question mark?
  void cmd_normal_input(shared_ptr<CompilerContext> context, bool questionMark);

 public:
  //! @brief INPUT statement
  bool execute(shared_ptr<CompilerContext> context) override;
  //! @brief LINE INPUT statement
  bool executeLineInput(shared_ptr<CompilerContext> context);
};

#endif  // COMPILER_INPUT_STATEMENT_STRATEGY_H_INCLUDED
