#include "compiler_call_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"

void CompilerCallStatementStrategy::cmd_call(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t = context->current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "TURBO") {
        // not need anymore... he he he...

      } else if (lexeme->value == "SYSTEM") {
        // ld hl, data address
        variable.addVarAddress(action);
        // ld a, (hl)      ; first character
        cpu.addLdAiHL();
        // ld ix, (CALL)    ; CALL
        cpu.addLdIXii(def_CALL_STMT);
        // call CALBAS
        cpu.addCall(def_CALBAS);
        // ei
        cpu.addEI();

      } else {
        // todo: concatenate CALL statement  into a literal string
        //       and run it with BASIC like above
        context->syntaxError("CALL statement invalid");
        return;
      }
    }

  } else {
    context->syntaxError("CALL with empty parameters");
  }
}

bool CompilerCallStatementStrategy::execute(CompilerContext* context) {
  cmd_call(context);
  return context->compiled;
}
