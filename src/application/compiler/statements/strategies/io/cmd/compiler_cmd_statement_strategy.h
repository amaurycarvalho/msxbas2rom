#ifndef COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED

#include <functional>
#include <memory>

#include "compiler_statement_strategy.h"

class CompilerCmdHandlerFactory;

using namespace std;

class CompilerCmdStatementStrategy : public ICompilerStatementStrategy {
 private:
  shared_ptr<CompilerCmdHandlerFactory> factory;

  void cmd_cmd(shared_ptr<CompilerContext> context);

 public:
  CompilerCmdStatementStrategy();
  ~CompilerCmdStatementStrategy();

  bool execute(shared_ptr<CompilerContext> context) override;
};

#endif  // COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED
