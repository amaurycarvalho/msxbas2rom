/***
 * @file compiler_cmd_plyload_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plyload_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdPlyLoadHandler::execute(CompilerContext* context,
                                        ActionNode* action) {
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
    }

    // ld (ARG), hl
    cpu.addLdiiHL(def_ARG);

  } else {
    context->syntaxError("CMD PLYLOAD syntax error");
  }

  // call cmd_plyload
  cpu.addCall(def_cmd_plyload);

  return context->compiled;
}
