#include "compiler_close_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerCloseStatementStrategy::cmd_close(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  context->file_support = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();
      // call cmd_fclose
      cpu.addCall(def_cmd_fclose);
    }

  } else {
    // CLOSE without parameters = CLOSE ALL (from 1 to MAXFIL)
    // ld a, 0xFF
    cpu.addXorA();
    // call cmd_fclose
    cpu.addCall(def_cmd_fclose);
  }
}

bool CompilerCloseStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_close(context);
  return context->compiled;
}
