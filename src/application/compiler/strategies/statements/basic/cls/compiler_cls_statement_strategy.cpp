#include "compiler_cls_statement_strategy.h"

bool CompilerClsStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::cls_stmt, ctx.traps_checked);
}
