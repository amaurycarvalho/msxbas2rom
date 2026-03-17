#include "compiler_cmd_rsctoram_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdRscToRamHandler::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 2 || action->actions.size() == 3) {
    auto sub = action->actions[0];  // resource number
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    sub = action->actions[1];  // ram dest address
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    if (action->actions.size() == 2) {
      // ex de, hl
      cpu.addExDEHL();
      // xor a   ; no pletter
      cpu.addXorA();
    } else {
      // push hl
      cpu.addPushHL();

      sub = action->actions[2];  // pletter? 0=no, 1=yes
      subtype = expression.evalExpression(sub);
      expression.addCast(subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();

      // pop de
      cpu.addPopDE();
    }

    // pop hl
    cpu.addPopHL();

    // call cmd_rsctoram
    cpu.addCall(def_cmd_rsctoram);

  } else {
    context->syntaxError("CMD RSCTORAM syntax error");
  }

  return context->compiled;
}
