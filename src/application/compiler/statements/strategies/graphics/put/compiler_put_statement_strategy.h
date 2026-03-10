#ifndef COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPutStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_put(CompilerContext* context);
  void cmd_put_sprite(CompilerContext* context);
  void cmd_put_tile(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED
