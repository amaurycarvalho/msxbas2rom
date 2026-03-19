#ifndef COMPILER_WIDTH_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_WIDTH_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerWidthStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_width(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_WIDTH_STATEMENT_STRATEGY_H_INCLUDED
