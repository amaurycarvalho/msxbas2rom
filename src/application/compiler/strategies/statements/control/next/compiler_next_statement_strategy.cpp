#include "compiler_next_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"

void CompilerNextStatementStrategy::cmd_next(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  ForNextNode* forNext;

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

bool CompilerNextStatementStrategy::execute(CompilerContext* context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_next(context);
  return context->compiled;
}
