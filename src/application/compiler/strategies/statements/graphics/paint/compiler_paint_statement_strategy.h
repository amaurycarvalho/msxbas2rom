#ifndef COMPILER_PAINT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_PAINT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerPaintStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_paint(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_PAINT_STATEMENT_STRATEGY_H_INCLUDED
