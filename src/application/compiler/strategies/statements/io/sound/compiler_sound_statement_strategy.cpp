#include "compiler_sound_statement_strategy.h"

bool CompilerSoundStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::sound_stmt, ctx.traps_checked);
}
