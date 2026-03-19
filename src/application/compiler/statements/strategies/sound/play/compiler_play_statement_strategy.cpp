#include "compiler_play_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerPlayStatementStrategy::cmd_play(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool xor_a = true;

  if (t >= 1 && t <= 3) {
    for (i = 0; i < 3; i++) {
      if (i < t) {
        action = context->current_action->actions[i];
        result_subtype = expression.evalExpression(action);
        if (result_subtype != Lexeme::subtype_string) {
          context->syntaxError("Invalid PLAY parameter");
          return;
        }
        // push hl
        cpu.addPushHL();
      } else {
        if (xor_a) {
          // xor a
          cpu.addXorA();
          xor_a = false;
        }
        // push af
        cpu.addPushAF();
      }
    }

    // pop bc
    cpu.addPopBC();
    // pop de
    cpu.addPopDE();
    // pop hl
    cpu.addPopHL();

    cpu.addCall(def_XBASIC_PLAY);

  } else {
    context->syntaxError("Invalid PLAY parameters");
  }
}

bool CompilerPlayStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_play(context);
  return context->compiled;
}
