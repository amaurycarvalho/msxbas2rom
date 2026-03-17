#include "compiler_cmd_play_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdPlayHandler::execute(CompilerContext* context,
                                     shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);

    // xor a
    cpu.addXorA();
    // ld (ARG), a
    cpu.addLdiiA(def_ARG);

    // call cmd_play
    cpu.addCall(def_cmd_play);

  } else if (action->actions.size() == 2) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);

    sub = action->actions[1];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (ARG), hl
    cpu.addLdiiHL(def_ARG);

    // call cmd_play
    cpu.addCall(def_cmd_play);

  } else {
    context->syntaxError("CMD PLAY syntax error");
  }

  return context->compiled;
}
