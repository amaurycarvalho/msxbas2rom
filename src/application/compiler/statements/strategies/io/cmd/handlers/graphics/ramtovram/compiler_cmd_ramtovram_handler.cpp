#include "compiler_cmd_ramtovram_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdRamToVramHandler::execute(CompilerContext* context,
                                          ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 3) {
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

    sub = action->actions[2];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (ARG2), hl
    cpu.addLdiiHL(def_ARG2);

    // call cmd_ramtovram
    cpu.addCall(def_cmd_ramtovram);

  } else {
    context->syntaxError("CMD RAMTOVRAM syntax error");
  }

  return context->compiled;
}
