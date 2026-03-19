#include "compiler_width_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerWidthStatementStrategy::cmd_width(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // call XBASIC WIDTH   ; xbasic WIDTH (in: l = size)
    cpu.addCall(def_XBASIC_WIDTH);

  } else {
    context->syntaxError("WIDTH syntax error");
  }
}

bool CompilerWidthStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_width(context);
  return context->compiled;
}
