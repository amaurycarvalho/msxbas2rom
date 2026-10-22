#include "compiler_close_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerCloseStatementStrategy::cmd_close(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->getCurrentAction()->actions.size();
  int result_subtype;

  context->getFileSupport() = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->getCurrentAction()->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();
      // call cmd_fclose
      context->getCodeOptimizer()->addKernelCall(DISP_cmd_fclose);
    }

  } else {
    // CLOSE without parameters = CLOSE ALL (from 1 to MAXFIL)
    // ld a, 0xFF
    cpu.addXorA();
    // call cmd_fclose
    context->getCodeOptimizer()->addKernelCall(DISP_cmd_fclose);
  }
}

bool CompilerCloseStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_close(context);
  return context->getCompiled();
}
