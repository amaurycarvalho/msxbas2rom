#ifndef COMPILER_CLS_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CLS_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerClsStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_cls(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_CLS_STATEMENT_STRATEGY_H_INCLUDED
