#include "compiler_irestore_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerIrestoreStatementStrategy::cmd_irestore(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, (DATLIN)
    cpu.addLdHLii(def_DATLIN);
    // ld (DATPTR), hl
    cpu.addLdiiHL(def_DATPTR);

  } else if (t == 1) {
    action = context->current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      // cast
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ex de, hl
      cpu.addExDEHL();
      // ld hl, (DATLIN)
      cpu.addLdHLii(def_DATLIN);
      // add hl, de
      cpu.addAddHLDE();
      // ld (DATPTR), hl
      cpu.addLdiiHL(def_DATPTR);
    }

  } else {
    context->syntaxError("IRESTORE with wrong number of parameters");
  }
}

bool CompilerIrestoreStatementStrategy::execute(CompilerContext* context) {
  cmd_irestore(context);
  return context->compiled;
}
