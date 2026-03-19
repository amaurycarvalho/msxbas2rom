#ifndef COMPILER_SCREEN_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SCREEN_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerScreenStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_screen(shared_ptr<CompilerContext> context);
  void cmd_screen_copy(shared_ptr<CompilerContext> context);
  void cmd_screen_paste(shared_ptr<CompilerContext> context);
  void cmd_screen_scroll(shared_ptr<CompilerContext> context);
  void cmd_screen_load(shared_ptr<CompilerContext> context);
  void cmd_screen_on(shared_ptr<CompilerContext> context);
  void cmd_screen_off(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_SCREEN_STATEMENT_STRATEGY_H_INCLUDED
