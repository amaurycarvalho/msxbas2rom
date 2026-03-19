/***
 * @file compiler_cmd_plyloop_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plyloop_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

bool CompilerCmdPlyLoopHandler::execute(shared_ptr<CompilerContext> context,
                                        shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  context->akm = true;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);
    // call cmd_plyloop
    cpu.addCall(def_cmd_plyloop);

  } else {
    context->syntaxError("CMD PLYLOOP syntax error");
  }

  return context->compiled;
}
