#ifndef COMPILER_TEXT_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_TEXT_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerTextStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_text(CompilerContext* context);

 public:
  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_TEXT_STATEMENT_STRATEGY_H_INCLUDED
