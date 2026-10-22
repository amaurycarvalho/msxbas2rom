#include "compiler_locate_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerLocateStatementStrategy::cmd_locate(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& optimizer = *context->getCodeOptimizer();
  shared_ptr<ActionNode> action;
  unsigned int t = context->getCurrentAction()->actions.size();
  int result_subtype;

  if (t != 2) {
    context->syntaxError("LOCATE without enough parameters");
  } else {
    action = context->getCurrentAction()->actions[0];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    action = context->getCurrentAction()->actions[1];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // pop de
    optimizer.addByteOptimized(0xD1);
    // call XBASIC_LOCATE    ; hl = y, de = x
    context->getCodeOptimizer()->addKernelCall(DISP_XBASIC_LOCATE);
  }
}

bool CompilerLocateStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_locate(context);
  return context->getCompiled();
}
