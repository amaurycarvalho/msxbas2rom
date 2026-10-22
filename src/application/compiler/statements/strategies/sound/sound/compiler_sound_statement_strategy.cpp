#include "compiler_sound_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerSoundStatementStrategy::cmd_sound(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->getCurrentAction()->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->getCurrentAction()->actions[i];
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
          context->getCodeOptimizer()->addKernelCall(DISP_XBASIC_SOUND);
        }
      }
    }

  } else {
    context->syntaxError("Invalid SOUND parameters");
  }
}

bool CompilerSoundStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_sound(context);
  return context->getCompiled();
}
