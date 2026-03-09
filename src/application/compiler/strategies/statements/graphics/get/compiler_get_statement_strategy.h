#ifndef COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerGetStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_get(CompilerContext* context);
  void cmd_get_date(CompilerContext* context);
  void cmd_get_time(CompilerContext* context);
  void cmd_get_tile(CompilerContext* context);
  void cmd_get_sprite(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_GET_STATEMENT_STRATEGY_H_INCLUDED
