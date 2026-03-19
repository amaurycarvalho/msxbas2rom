#ifndef COMPILER_DRAW_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_DRAW_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerDrawStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_draw(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_DRAW_STATEMENT_STRATEGY_H_INCLUDED
