#ifndef COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGetStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_get(shared_ptr<CompilerContext> context);
  void cmd_get_date(shared_ptr<CompilerContext> context);
  void cmd_get_time(shared_ptr<CompilerContext> context);
  void cmd_get_tile(shared_ptr<CompilerContext> context);
  void cmd_get_sprite(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED
