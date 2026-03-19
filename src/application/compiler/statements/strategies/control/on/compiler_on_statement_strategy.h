#ifndef COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerOnStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_on(shared_ptr<CompilerContext> context);
  void cmd_on_error(shared_ptr<CompilerContext> context);
  void cmd_on_interval(shared_ptr<CompilerContext> context);
  void cmd_on_key(shared_ptr<CompilerContext> context);
  void cmd_on_sprite(shared_ptr<CompilerContext> context);
  void cmd_on_stop(shared_ptr<CompilerContext> context);
  void cmd_on_strig(shared_ptr<CompilerContext> context);
  void cmd_on_goto_gosub(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_ON_STATEMENT_STRATEGY_H_INCLUDED
