#ifndef COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED

#include <functional>
#include <memory>

#include "compiler_statement_strategy.h"

class CompilerCmdHandlerFactory;

using namespace std;

class CompilerCmdStatementStrategy : public ICompilerStatementStrategy {
 private:
  unique_ptr<CompilerCmdHandlerFactory> factory;

  void cmd_cmd(CompilerContext* context);

 public:
  CompilerCmdStatementStrategy();
  ~CompilerCmdStatementStrategy();

  bool execute(CompilerContext* context) override;
};

#endif  // COMPILER_CMD_STATEMENT_STRATEGY_H_INCLUDED
