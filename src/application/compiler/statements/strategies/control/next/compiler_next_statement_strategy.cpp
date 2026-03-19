#include "compiler_next_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "fix_node.h"
#include "for_next_node.h"

void CompilerNextStatementStrategy::cmd_next(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  shared_ptr<ForNextNode> forNext;

  if (context->forNextStack.size()) {
    forNext = context->forNextStack.top();
    context->forNextStack.pop();

    // jp step
    fixup.addFix(forNext->for_step_mark);
    cpu.addJp(0x0000);

    if (forNext->for_end_mark)
      forNext->for_end_mark->symbol->address = cpu.context->code_pointer;

  } else {
    context->syntaxError("NEXT without a FOR");
  }
}

bool CompilerNextStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_next(context);
  return context->compiled;
}
