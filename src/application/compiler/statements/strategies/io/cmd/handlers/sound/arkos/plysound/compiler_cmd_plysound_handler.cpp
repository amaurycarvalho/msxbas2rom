/***
 * @file compiler_cmd_plysound_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plysound_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

bool CompilerCmdPlySoundHandler::execute(shared_ptr<CompilerContext> context,
                                         shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  context->akm = true;

  if (action->actions.size()) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);

    if (action->actions.size() > 1) {
      sub = action->actions[1];
      subtype = expression.evalExpression(sub);
      expression.addCast(subtype, Lexeme::subtype_numeric);
    } else {
      // ld hl, 0
      cpu.addLdHL(0x0000);
    }

    // ld (ARG), hl
    cpu.addLdiiHL(def_ARG);

    if (action->actions.size() > 2) {
      sub = action->actions[1];
      subtype = expression.evalExpression(sub);
      expression.addCast(subtype, Lexeme::subtype_numeric);
    } else if (action->actions.size() > 1) {
      // ld hl, 0
      cpu.addLdHL(0x0000);
    }

    // ld (ARG+2), hl
    cpu.addLdiiHL(def_ARG + 2);

  } else {
    context->syntaxError("CMD PLYSOUND syntax error");
  }

  // call cmd_plysound
  cpu.addCall(def_cmd_plysound);

  return context->compiled;
}
