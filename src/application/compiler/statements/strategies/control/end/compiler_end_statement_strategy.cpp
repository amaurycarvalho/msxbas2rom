#include "compiler_end_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "parser.h"

void CompilerEndStatementStrategy::cmd_end(shared_ptr<CompilerContext> context,
                                           bool doCodeRegistering) {
  auto& cpu = *context->getCpu();
  auto& fixup = *context->getFixupResolver();
  auto& parser = *context->getParser();
  if (doCodeRegistering) {
    /// @remark first instruction needs to be a skip to the program start code
    if (parser.getHasAkm()) {
      cpu.addJr(1 + 3 + 10);
    } else
      cpu.addJr(1 + 10);

    /// mark the END statement start code
    context->getEndMark() = fixup.addPreMark();
    context->getEndMark()->address = cpu.context->getCodePointer();

    /// write the END statement code
    if (parser.getHasAkm()) {
      // disable akm player
      context->getCodeOptimizer()->addKernelCall(DISP_player_unhook);
    }

    // call XBASIC_END
    context->getCodeOptimizer()->addKernelCall(DISP_XBASIC_END);

    // ld sp, (SAVSTK)
    cpu.addLdSPii(0xf6b1);

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // ret               ; return to basic
    cpu.addRet();

  } else {
    /// jump to the real END statement
    // jp ctx.end_mark
    if (context->getEndMark()) {
      fixup.addFix(context->getEndMark());
      cpu.addJp(0x0000);
    }
  }
}

bool CompilerEndStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  context->getTrapsChecked() = context->getCodeHelper()->addCheckTraps();
  //! jump to the real END statement
  cmd_end(context, false);
  return context->getCompiled();
}

bool CompilerEndStatementStrategy::registerEndRoutine(
    shared_ptr<CompilerContext> context) {
  // context->getTrapsChecked() = context->getCodeHelper()->addCheckTraps();
  //! register END statement code
  cmd_end(context, true);
  return context->getCompiled();
}
