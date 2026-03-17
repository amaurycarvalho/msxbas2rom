#include "compiler_vpoke_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerVpokeStatementStrategy::cmd_vpoke(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          cpu.addPushHL();
        } else {
          // ld a, l
          cpu.addLdAL();
          // pop hl
          cpu.addPopHL();
          // call 0x70b5    ; xbasic VPOKE (in: hl=address, a=byte)
          cpu.addCall(def_XBASIC_VPOKE);
        }
      }
    }

  } else {
    context->syntaxError("Invalid VPOKE parameters");
  }
}

bool CompilerVpokeStatementStrategy::execute(CompilerContext* context) {
  cmd_vpoke(context);
  return context->compiled;
}
