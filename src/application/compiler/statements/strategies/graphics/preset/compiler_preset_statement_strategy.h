#ifndef COMPILER_PRESET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PRESET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_pset_statement_strategy.h"

class CompilerPresetStatementStrategy : public CompilerPsetStatementStrategy {
 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_PRESET_STATEMENT_STRATEGY_H_INCLUDED
