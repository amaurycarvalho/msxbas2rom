#include "compiler_next_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "fix_node.h"
#include "for_next_node.h"

void CompilerNextStatementStrategy::cmd_next(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& fixup = *context->getFixupResolver();
  shared_ptr<ForNextNode> forNext;

  if (context->getForNextStack().size()) {
    forNext = context->getForNextStack().top();
    context->getForNextStack().pop();

    // jp step
    fixup.addFix(forNext->forStepMark);
    cpu.addJp(0x0000);

    if (forNext->forEndMark) forNext->forEndMark->aimHere();

  } else {
    context->syntaxError("NEXT without a FOR");
  }
}

bool CompilerNextStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  context->getTrapsChecked() = context->getCodeHelper()->addCheckTraps();
  cmd_next(context);
  return context->getCompiled();
}
