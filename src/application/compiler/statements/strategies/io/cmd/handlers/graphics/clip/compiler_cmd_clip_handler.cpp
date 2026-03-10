#include "compiler_cmd_clip_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

bool CompilerCmdClipHandler::execute(CompilerContext* context,
                                     ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (action->actions.size() == 1) {
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    // ld a, l
    cpu.addLdAL();
    // ld hl, 0xfafc
    cpu.addLdHL(0xfafc);
    // res 4, (hl)
    cpu.addWord(0xCB, 0xA6);
    // bit 0, a
    cpu.addWord(0xCB, 0x47);
    // jr z, +3
    cpu.addJrZ(0x02);
    //   set 4, (hl)
    cpu.addWord(0xCB, 0xE6);

  } else {
    context->syntaxError("CMD CLIP syntax error");
  }

  return context->compiled;
}
