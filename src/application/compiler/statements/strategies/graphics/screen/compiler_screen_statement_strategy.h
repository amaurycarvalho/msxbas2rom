#ifndef COMPILER_SCREEN_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SCREEN_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerScreenStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_screen(CompilerContext* context);
  void cmd_screen_copy(CompilerContext* context);
  void cmd_screen_paste(CompilerContext* context);
  void cmd_screen_scroll(CompilerContext* context);
  void cmd_screen_load(CompilerContext* context);
  void cmd_screen_on(CompilerContext* context);
  void cmd_screen_off(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_SCREEN_STATEMENT_STRATEGY_H_INCLUDED
