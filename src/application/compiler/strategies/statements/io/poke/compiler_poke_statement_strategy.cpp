#include "compiler_poke_statement_strategy.h"

bool CompilerPokeStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::poke_stmt, ctx.traps_checked);
}
