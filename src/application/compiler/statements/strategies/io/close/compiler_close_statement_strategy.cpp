#include "compiler_close_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerCloseStatementStrategy::cmd_close(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  auto& fixup = *context->fixupResolver;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  shared_ptr<FixNode> skipCloseMark;

  context->file_support = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();
      // push af
      cpu.addPushAF();

      codeHelper.addEnableBasicSlot();

      // pop af
      cpu.addPopAF();
      // ld (TEMP), a           ; preserve i/o number
      cpu.addLdiiA(def_TEMP);
      // ld a, 0                ; drive A:
      cpu.addLdA(0x00);
      // call preflight disk
      cpu.addCall(def_cmd_preflight_disk);
      // and a
      cpu.addAndA();
      // jp nz, skip CLOSE
      skipCloseMark = fixup.addMark();
      cpu.addJpNZ(0x0000);
      // ld a, (TEMP)
      cpu.addLdAii(def_TEMP);
      // ld hl, fake empty line
      cpu.addLdHL(def_ENDPRG);

      // call CLOSE    ; in: a = i/o number
      cpu.addCall(def_CLOSE);
      skipCloseMark->symbol->address = cpu.context->code_pointer;

      codeHelper.addDisableBasicSlot();
    }

  } else {
    codeHelper.addEnableBasicSlot();

    // ld a, 0                ; drive A:
    cpu.addLdA(0x00);
    // call preflight disk
    cpu.addCall(def_cmd_preflight_disk);
    // and a
    cpu.addAndA();
    // jp nz, skip CLOSE ALL
    skipCloseMark = fixup.addMark();
    cpu.addJpNZ(0x0000);

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // call ClOSE ALL
    cpu.addCall(def_CLOSE_ALL);
    skipCloseMark->symbol->address = cpu.context->code_pointer;

    codeHelper.addDisableBasicSlot();
  }
}

bool CompilerCloseStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_close(context);
  return context->compiled;
}
