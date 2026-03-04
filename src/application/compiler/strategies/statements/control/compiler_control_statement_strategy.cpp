#include "compiler_control_statement_strategy.h"

#include <string>

#include "lexeme.h"

using namespace std;

bool ControlCompilerStatementStrategy::execute(CompilerStatementContext& ctx) {
  const string& keyword = ctx.lexeme->name;
  CompilerCommandId command;

  if (keyword == "END")
    command = CompilerCommandId::end_stmt;
  else if (keyword == "GOTO")
    command = CompilerCommandId::goto_stmt;
  else if (keyword == "GOSUB")
    command = CompilerCommandId::gosub_stmt;
  else if (keyword == "RETURN")
    command = CompilerCommandId::return_stmt;
  else if (keyword == "IF")
    command = CompilerCommandId::if_stmt;
  else if (keyword == "FOR")
    command = CompilerCommandId::for_stmt;
  else if (keyword == "NEXT")
    command = CompilerCommandId::next_stmt;
  else if (keyword == "ON")
    command = CompilerCommandId::on_stmt;
  else if (keyword == "INTERVAL")
    command = CompilerCommandId::interval_stmt;
  else if (keyword == "STOP")
    command = CompilerCommandId::stop_stmt;
  else
    return false;

  return ctx.dispatch(command, ctx.traps_checked);
}
