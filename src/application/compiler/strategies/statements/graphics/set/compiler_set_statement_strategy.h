#ifndef COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerSetStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_set(CompilerContext* context);
  void cmd_set_video(CompilerContext* context);
  void cmd_set_adjust(CompilerContext* context);
  void cmd_set_screen(CompilerContext* context);
  void cmd_set_beep(CompilerContext* context);
  void cmd_set_title(CompilerContext* context);
  void cmd_set_prompt(CompilerContext* context);
  void cmd_set_page(CompilerContext* context);
  void cmd_set_scroll(CompilerContext* context);
  void cmd_set_tile(CompilerContext* context);
  void cmd_set_font(CompilerContext* context);
  void cmd_set_sprite(CompilerContext* context);
  void cmd_set_date(CompilerContext* context);
  void cmd_set_time(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_SET_STATEMENT_STRATEGY_H_INCLUDED
