#include "compiler_cmd_wrtfnt_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdWrtFntHandler::execute(CompilerContext* context,
                                       ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() != 1) {
    context->syntaxError("CMD WRTFNT syntax error");
    return false;
  }

  auto sub = action->actions[0];
  int subtype = expression.evalExpression(sub);
  expression.addCast(subtype, Lexeme::subtype_numeric);

  // ld (DAC), hl
  cpu.addLdiiHL(def_DAC);
  // call cmd_wrtfnt
  cpu.addCall(def_cmd_wrtfnt);

  return context->compiled;
}
