#include "compiler_basic_statement_strategy.h"

#include <string>

#include "lexeme.h"

using namespace std;

bool BasicCompilerStatementStrategy::execute(CompilerStatementContext& ctx) {
  const string& keyword = ctx.lexeme->name;
  CompilerCommandId command;

  if (keyword == "CLEAR")
    command = CompilerCommandId::clear_stmt;
  else if (keyword == "DEF" || keyword == "DEFINT" || keyword == "DEFSNG" ||
           keyword == "DEFDBL" || keyword == "DEFSTR")
    command = CompilerCommandId::def_stmt;
  else if (keyword == "CLS")
    command = CompilerCommandId::cls_stmt;
  else if (keyword == "BEEP")
    command = CompilerCommandId::beep_stmt;
  else if (keyword == "LET" || keyword == "TIME")
    command = CompilerCommandId::let_stmt;
  else if (keyword == "DIM")
    command = CompilerCommandId::dim_stmt;
  else if (keyword == "REDIM")
    command = CompilerCommandId::redim_stmt;
  else if (keyword == "RANDOMIZE")
    command = CompilerCommandId::randomize_stmt;
  else
    return false;

  return ctx.dispatch(command, ctx.traps_checked);
}
