#include "compiler_maxfiles_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"

void CompilerMaxfilesStatementStrategy::cmd_maxfiles(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;
  FixNode* mark;

  if (t) {
    // xor a
    // cpu.addXorA();
    // ld (NLONLY), a
    // cpu.addLdiiA(def_NLONLY);
    // ld (DSKDIS), a
    // cpu.addLdiiA(0xFD99);

    action = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // ld a, (MAXFIL)
    cpu.addLdAii(def_MAXFIL);
    // cp l
    cpu.addCpL();
    // jp z, $                ; skip if equal
    mark = fixup.addMark();
    cpu.addJpZ(0x0000);

    // ld a, l
    cpu.addLdAL();

    // ld (MAXFIL), a
    cpu.addLdiiA(def_MAXFIL);

    // push af
    cpu.addPushAF();

    codeHelper.addEnableBasicSlot();

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // call CLOSE_ALL
    cpu.addCall(def_CLOSE_ALL);

    // pop af
    cpu.addPopAF();

    // call MAXFILES
    cpu.addCall(def_MAXFILES);

    // call 0x7304			; end printeroutput (basic interpreter
    // function)
    cpu.addCall(0x7304);
    // call 0x4AFF			; return interpreter output to screen
    // (basic interpreter function)
    cpu.addCall(0x4AFF);

    codeHelper.addDisableBasicSlot();

    // ; restore stack state
    // ld bc, 16
    cpu.addLdBC(0x0010);
    // ld (TEMP), sp
    cpu.addLdiiSP(def_TEMP);
    // ld hl, (TEMP)
    cpu.addLdHLii(def_TEMP);
    // xor a
    cpu.addXorA();
    // sbc hl, bc
    cpu.addSbcHLBC();
    // ld sp,hl
    cpu.addLdSPHL();
    // ex de,hl
    cpu.addExDEHL();
    // ld hl, (SAVSTK)
    cpu.addLdHLii(def_SAVSTK);
    // ldir
    cpu.addLDIR();
    // ld (SAVSTK), sp
    cpu.addLdiiSP(def_SAVSTK);

    mark->symbol->address = cpu.context->code_pointer;

  } else {
    context->syntaxError("Empty MAXFILES assignment");
  }
}

bool CompilerMaxfilesStatementStrategy::execute(CompilerContext* context) {
  cmd_maxfiles(context);
  return context->compiled;
}
