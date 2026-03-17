#include "compiler_sound_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerSoundStatementStrategy::cmd_sound(CompilerContext* context) {
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
          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();
        } else {
          // ld e, l
          cpu.addLdEL();
          // pop af
          cpu.addPopAF();

          // call sound function
          cpu.addCall(def_XBASIC_SOUND);
        }
      }
    }

  } else {
    context->syntaxError("Invalid SOUND parameters");
  }
}

bool CompilerSoundStatementStrategy::execute(CompilerContext* context) {
  cmd_sound(context);
  return context->compiled;
}
