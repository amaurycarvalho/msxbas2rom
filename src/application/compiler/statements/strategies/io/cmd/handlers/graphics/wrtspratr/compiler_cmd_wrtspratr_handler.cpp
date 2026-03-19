#include "compiler_cmd_wrtspratr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

bool CompilerCmdWrtSprAtrHandler::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);
    // call cmd_wrtspratr
    cpu.addCall(def_cmd_wrtspratr);

  } else {
    context->syntaxError("CMD WRTSPRATR syntax error");
  }

  return context->compiled;
}
