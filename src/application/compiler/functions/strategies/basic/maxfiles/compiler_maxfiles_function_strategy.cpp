#include "compiler_maxfiles_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int MaxfilesCompilerFunctionStrategy::execute(
    shared_ptr<CompilerContext> context, shared_ptr<ActionNode> action,
    int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "MAXFILES") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // ld hl, (MAXFIL)
  cpu.addLdHLii(def_MAXFIL);

  return Lexeme::subtype_numeric;
}
