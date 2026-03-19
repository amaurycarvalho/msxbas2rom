#include "compiler_restore_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerRestoreStatementStrategy::cmd_restore(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, 0
    cpu.addLdHL(0x0000);
    // call xbasic_restore
    cpu.addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement

  } else if (t == 1) {
    action = context->current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      // cast
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // call restore
      cpu.addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement
    }

  } else {
    context->syntaxError("RESTORE with wrong number of parameters");
  }
}

bool CompilerRestoreStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_restore(context);
  return context->compiled;
}
