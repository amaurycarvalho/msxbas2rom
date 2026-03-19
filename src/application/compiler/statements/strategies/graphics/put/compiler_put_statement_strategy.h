#ifndef COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPutStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_put(shared_ptr<CompilerContext> context);
  void cmd_put_sprite(shared_ptr<CompilerContext> context);
  void cmd_put_tile(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_PUT_STATEMENT_STRATEGY_H_INCLUDED
