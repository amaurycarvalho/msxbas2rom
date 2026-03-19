#include "compiler_cmd_wrtvram_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

bool CompilerCmdWrtVramHandler::execute(shared_ptr<CompilerContext> context,
                                        shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() != 2) {
    context->syntaxError("CMD WRTVRAM syntax error");
    return false;
  }

  auto sub1 = action->actions[0];
  int subtype = expression.evalExpression(sub1);
  expression.addCast(subtype, Lexeme::subtype_numeric);

  // ld (DAC), hl
  cpu.addLdiiHL(def_DAC);

  auto sub2 = action->actions[1];
  subtype = expression.evalExpression(sub2);
  expression.addCast(subtype, Lexeme::subtype_numeric);

  // ld (ARG), hl
  cpu.addLdiiHL(def_ARG);

  // call cmd_runasm
  cpu.addCall(def_cmd_wrtvram);

  return context->compiled;
}
