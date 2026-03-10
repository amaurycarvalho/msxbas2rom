#ifndef COMPILER_PSET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PSET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPsetStatementStrategy : public ICompilerStatementStrategy {
 protected:
  //! @brief PSET and PRESET statement
  //! @param context Compiler context
  //! @param forecolor True for PSET, False for PRESET
  void cmd_pset(CompilerContext* context, bool forecolor);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PSET_STATEMENT_STRATEGY_H_INCLUDED
