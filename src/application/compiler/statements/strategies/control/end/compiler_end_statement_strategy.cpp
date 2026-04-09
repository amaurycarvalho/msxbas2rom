#include "compiler_end_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "parser.h"

void CompilerEndStatementStrategy::cmd_end(shared_ptr<CompilerContext> context,
                                           bool doCodeRegistering) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& parser = *context->parser;
  if (doCodeRegistering) {
    /// @remark first instruction needs to be a skip to the program start code
    if (parser.getHasAkm()) {
      cpu.addJr(1 + 3 + 10);
    } else
      cpu.addJr(1 + 10);

    /// mark the END statement start code
    context->end_mark = fixup.addPreMark();
    context->end_mark->address = cpu.context->code_pointer;

    /// write the END statement code
    if (parser.getHasAkm()) {
      // disable akm player
      cpu.addCall(def_player_unhook);
    }

    // call XBASIC_END
    cpu.addCall(def_XBASIC_END);

    // ld sp, (SAVSTK)
    cpu.addLdSPii(0xf6b1);

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // ret               ; return to basic
    cpu.addRet();

  } else {
    /// jump to the real END statement
    // jp ctx.end_mark
    if (context->end_mark) {
      fixup.addFix(context->end_mark);
      cpu.addJp(0x0000);
    }
  }
}

bool CompilerEndStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  //! jump to the real END statement
  cmd_end(context, false);
  return context->compiled;
}

bool CompilerEndStatementStrategy::registerEndRoutine(
    shared_ptr<CompilerContext> context) {
  // context->traps_checked = context->codeHelper->addCheckTraps();
  //! register END statement code
  cmd_end(context, true);
  return context->compiled;
}
