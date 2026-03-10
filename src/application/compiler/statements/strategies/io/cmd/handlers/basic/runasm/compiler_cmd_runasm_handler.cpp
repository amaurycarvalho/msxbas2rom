#include "compiler_cmd_runasm_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdRunAsmHandler::execute(CompilerContext* context,
                                       ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() != 1) {
    context->syntaxError("CMD RUNASM syntax error");
    return false;
  }

  auto sub = action->actions[0];

  int subtype = expression.evalExpression(sub);
  expression.addCast(subtype, Lexeme::subtype_numeric);

  // ld (DAC), hl
  cpu.addLdiiHL(def_DAC);
  // call cmd_runasm
  cpu.addCall(def_cmd_runasm);

  return context->compiled;
}
