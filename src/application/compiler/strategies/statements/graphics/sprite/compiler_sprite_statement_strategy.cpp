#include "compiler_sprite_statement_strategy.h"

bool CompilerSpriteStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::sprite_stmt, ctx.traps_checked);
}
