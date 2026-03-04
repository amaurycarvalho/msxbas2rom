#ifndef COMPILER_PRESET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PRESET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPresetStatementStrategy : public ICompilerStatementStrategy {
 public:
  bool execute(CompilerStatementContext& ctx) override;
};

#endif  // COMPILER_PRESET_STATEMENT_STRATEGY_H_INCLUDED
