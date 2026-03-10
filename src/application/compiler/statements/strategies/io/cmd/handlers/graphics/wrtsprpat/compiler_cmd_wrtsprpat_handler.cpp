#include "compiler_cmd_wrtsprpat_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdWrtSprPatHandler::execute(CompilerContext* context,
                                          ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);
    // call cmd_wrtsprpat
    cpu.addCall(def_cmd_wrtsprpat);

  } else {
    context->syntaxError("CMD WRTSPRPAT syntax error");
  }

  return context->compiled;
}
