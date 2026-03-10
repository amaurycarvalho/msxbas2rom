#include "compiler_cmd_page_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdPageHandler::execute(CompilerContext* context,
                                     ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld de, 0                 ; delay = stop
    cpu.addLdDE(0);

    // push de
    cpu.addPushDE();

    // pop bc
    cpu.addPopBC();

  } else if (action->actions.size() == 2) {
    auto sub = action->actions[1];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();
    // push hl
    cpu.addPushHL();

    sub = action->actions[0];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // pop de
    cpu.addPopDE();

    // pop bc
    cpu.addPopBC();

  } else if (action->actions.size() == 3) {
    auto sub = action->actions[2];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    sub = action->actions[1];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    sub = action->actions[0];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // pop de
    cpu.addPopDE();

    // pop bc
    cpu.addPopBC();

  } else {
    context->syntaxError("CMD PAGE syntax error");
  }

  // call cmd_page (l = mode, e = delay #1, c = delay #2)
  cpu.addCall(def_cmd_page);

  return context->compiled;
}
