#include "compiler_cmd_setfnt_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdSetFntHandler::execute(CompilerContext* context,
                                       shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  context->font = true;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);

    // ld a, 0xff                ; it means all screen banks
    cpu.addLdA(0xFF);
    // ld (ARG), a
    cpu.addLdiiA(def_ARG);

    // call cmd_setfnt
    cpu.addCall(def_cmd_setfnt);

  } else if (action->actions.size() == 2) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);

    sub = action->actions[1];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (ARG), hl             ; screen font bank number
    cpu.addLdiiHL(def_ARG);

    // call cmd_setfnt
    cpu.addCall(def_cmd_setfnt);

  } else {
    context->syntaxError("CMD SETFNT syntax error");
  }

  return context->compiled;
}
