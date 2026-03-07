#ifndef COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerOnStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_on(CompilerContext* context);
  void cmd_on_error(CompilerContext* context);
  void cmd_on_interval(CompilerContext* context);
  void cmd_on_key(CompilerContext* context);
  void cmd_on_sprite(CompilerContext* context);
  void cmd_on_stop(CompilerContext* context);
  void cmd_on_strig(CompilerContext* context);
  void cmd_on_goto_gosub(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED
