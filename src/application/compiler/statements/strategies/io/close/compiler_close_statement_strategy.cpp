#include "compiler_close_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerCloseStatementStrategy::cmd_close(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();
      // push af
      cpu.addPushAF();

      codeHelper.addEnableBasicSlot();

      // pop af
      cpu.addPopAF();
      // ld hl, fake empty line
      cpu.addLdHL(def_ENDPRG);

      // call CLOSE    ; in: a = i/o number
      cpu.addCall(def_CLOSE);

      codeHelper.addDisableBasicSlot();
    }

  } else {
    codeHelper.addEnableBasicSlot();

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // call ClOSE ALL
    cpu.addCall(def_CLOSE_ALL);

    codeHelper.addDisableBasicSlot();
  }
}

bool CompilerCloseStatementStrategy::execute(CompilerContext* context) {
  cmd_close(context);
  return context->compiled;
}
