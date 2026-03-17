#include "compiler_msx_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int MsxCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "MSX") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // ld hl, (VERSION)                    ; 0 = MSX1, 1 = MSX2, 2 = MSX2+, 3 =
  // MSXturboR
  cpu.addLdHLii(def_VERSION);
  // ld h, 0
  cpu.addLdH(0x00);

  return Lexeme::subtype_numeric;
}
