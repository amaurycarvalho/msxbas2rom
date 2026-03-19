#ifndef COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSetStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_set(shared_ptr<CompilerContext> context);
  void cmd_set_video(shared_ptr<CompilerContext> context);
  void cmd_set_adjust(shared_ptr<CompilerContext> context);
  void cmd_set_screen(shared_ptr<CompilerContext> context);
  void cmd_set_beep(shared_ptr<CompilerContext> context);
  void cmd_set_title(shared_ptr<CompilerContext> context);
  void cmd_set_prompt(shared_ptr<CompilerContext> context);
  void cmd_set_page(shared_ptr<CompilerContext> context);
  void cmd_set_scroll(shared_ptr<CompilerContext> context);
  void cmd_set_tile(shared_ptr<CompilerContext> context);
  void cmd_set_font(shared_ptr<CompilerContext> context);
  void cmd_set_sprite(shared_ptr<CompilerContext> context);
  void cmd_set_date(shared_ptr<CompilerContext> context);
  void cmd_set_time(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED
