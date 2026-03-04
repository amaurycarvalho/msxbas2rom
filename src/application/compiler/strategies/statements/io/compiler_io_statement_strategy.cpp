#include "compiler_io_statement_strategy.h"

#include <string>

#include "lexeme.h"

using namespace std;

bool IoCompilerStatementStrategy::execute(CompilerStatementContext& ctx) {
  const string& keyword = ctx.lexeme->name;
  CompilerCommandId command;

  if (keyword == "PRINT")
    command = CompilerCommandId::print_stmt;
  else if (keyword == "INPUT")
    command = CompilerCommandId::input_stmt;
  else if (keyword == "SOUND")
    command = CompilerCommandId::sound_stmt;
  else if (keyword == "OUT")
    command = CompilerCommandId::out_stmt;
  else if (keyword == "POKE")
    command = CompilerCommandId::poke_stmt;
  else if (keyword == "VPOKE")
    command = CompilerCommandId::vpoke_stmt;
  else if (keyword == "IPOKE")
    command = CompilerCommandId::ipoke_stmt;
  else if (keyword == "DATA")
    command = CompilerCommandId::data_stmt;
  else if (keyword == "IDATA")
    command = CompilerCommandId::idata_stmt;
  else if (keyword == "READ")
    command = CompilerCommandId::read_stmt;
  else if (keyword == "IREAD")
    command = CompilerCommandId::iread_stmt;
  else if (keyword == "RESTORE")
    command = CompilerCommandId::restore_stmt;
  else if (keyword == "IRESTORE")
    command = CompilerCommandId::irestore_stmt;
  else if (keyword == "RESUME")
    command = CompilerCommandId::resume_stmt;
  else if (keyword == "WAIT")
    command = CompilerCommandId::wait_stmt;
  else if (keyword == "SWAP")
    command = CompilerCommandId::swap_stmt;
  else if (keyword == "CALL")
    command = CompilerCommandId::call_stmt;
  else if (keyword == "CMD")
    command = CompilerCommandId::cmd_stmt;
  else if (keyword == "MAXFILES")
    command = CompilerCommandId::maxfiles_stmt;
  else if (keyword == "OPEN")
    command = CompilerCommandId::open_stmt;
  else if (keyword == "OPEN_GRP")
    command = CompilerCommandId::open_grp_stmt;
  else if (keyword == "CLOSE")
    command = CompilerCommandId::close_stmt;
  else if (keyword == "FILE")
    command = CompilerCommandId::file_stmt;
  else if (keyword == "TEXT")
    command = CompilerCommandId::text_stmt;
  else if (keyword == "BLOAD")
    command = CompilerCommandId::bload_stmt;
  else
    return false;

  return ctx.dispatch(command, ctx.traps_checked);
}
