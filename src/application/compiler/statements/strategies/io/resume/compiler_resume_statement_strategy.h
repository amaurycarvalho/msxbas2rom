#ifndef COMPILER_RESUME_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_RESUME_STATEMENT_STRATEGY_H_INCLUDED

#include "compiler_statement_strategy.h"

class CompilerResumeStatementStrategy : public ICompilerStatementStrategy {
 private:
  void cmd_resume(shared_ptr<CompilerContext> context);

 public:
  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_RESUME_STATEMENT_STRATEGY_H_INCLUDED
