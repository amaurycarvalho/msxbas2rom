#include "compiler_locate_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerLocateStatementStrategy::cmd_locate(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t != 2) {
    context->syntaxError("LOCATE without enough parameters");
  } else {
    action = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    action = context->current_action->actions[1];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // pop de
    optimizer.addByteOptimized(0xD1);
    // call XBASIC_LOCATE    ; hl = y, de = x
    cpu.addCall(def_XBASIC_LOCATE);
  }
}

bool CompilerLocateStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_locate(context);
  return context->compiled;
}
