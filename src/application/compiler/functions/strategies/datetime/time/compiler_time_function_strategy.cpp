#include "compiler_time_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int TimeCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "TIME") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // ld hl, (0xFC9E)    ; JIFFY
  cpu.addLdHLii(def_JIFFY);

  return Lexeme::subtype_numeric;
}
