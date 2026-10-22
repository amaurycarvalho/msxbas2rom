#include "compiler_cmd_runasm_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

bool CompilerCmdRunAsmHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();

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
  context->getCodeOptimizer()->addKernelCall(DISP_cmd_runasm);

  return context->getCompiled();
}
