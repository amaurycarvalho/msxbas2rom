#ifndef COMPILER_SOUND_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SOUND_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSoundStatementStrategy : public ICompilerStatementStrategy {
 private:
  /***
   * @brief SOUND statement
   * @note
   * https://www.msx.org/wiki/SOUND
   */
  void cmd_sound(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_SOUND_STATEMENT_STRATEGY_H_INCLUDED
