#ifndef COMPILER_SOUND_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SOUND_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSoundStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_sound(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_SOUND_STATEMENT_STRATEGY_H_INCLUDED
