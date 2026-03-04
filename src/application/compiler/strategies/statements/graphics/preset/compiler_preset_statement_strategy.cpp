#include "compiler_preset_statement_strategy.h"

bool CompilerPresetStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::preset_stmt, ctx.traps_checked);
}
