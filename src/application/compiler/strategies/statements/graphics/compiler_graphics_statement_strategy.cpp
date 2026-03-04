#include "compiler_graphics_statement_strategy.h"

#include <string>

#include "lexeme.h"

using namespace std;

bool GraphicsCompilerStatementStrategy::execute(CompilerStatementContext& ctx) {
  const string& keyword = ctx.lexeme->name;
  CompilerCommandId command;

  if (keyword == "PLAY")
    command = CompilerCommandId::play_stmt;
  else if (keyword == "DRAW")
    command = CompilerCommandId::draw_stmt;
  else if (keyword == "LOCATE")
    command = CompilerCommandId::locate_stmt;
  else if (keyword == "SCREEN")
    command = CompilerCommandId::screen_stmt;
  else if (keyword == "WIDTH")
    command = CompilerCommandId::width_stmt;
  else if (keyword == "COLOR")
    command = CompilerCommandId::color_stmt;
  else if (keyword == "PSET")
    command = CompilerCommandId::pset_stmt;
  else if (keyword == "PRESET")
    command = CompilerCommandId::preset_stmt;
  else if (keyword == "LINE")
    command = CompilerCommandId::line_stmt;
  else if (keyword == "PAINT")
    command = CompilerCommandId::paint_stmt;
  else if (keyword == "CIRCLE")
    command = CompilerCommandId::circle_stmt;
  else if (keyword == "COPY")
    command = CompilerCommandId::copy_stmt;
  else if (keyword == "PUT")
    command = CompilerCommandId::put_stmt;
  else if (keyword == "GET")
    command = CompilerCommandId::get_stmt;
  else if (keyword == "SET")
    command = CompilerCommandId::set_stmt;
  else if (keyword == "KEY")
    command = CompilerCommandId::key_stmt;
  else if (keyword == "STRIG")
    command = CompilerCommandId::strig_stmt;
  else if (keyword == "SPRITE")
    command = CompilerCommandId::sprite_stmt;
  else
    return false;

  return ctx.dispatch(command, ctx.traps_checked);
}
