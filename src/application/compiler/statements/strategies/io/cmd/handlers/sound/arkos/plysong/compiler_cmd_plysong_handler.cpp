/***
 * @file compiler_cmd_plysong_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plysong_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdPlySongHandler::execute(CompilerContext* context,
                                        ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  context->akm = true;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);

  } else {
    context->syntaxError("CMD PLYSONG syntax error");
  }

  // call cmd_plysong
  cpu.addCall(def_cmd_plysong);

  return context->compiled;
}
