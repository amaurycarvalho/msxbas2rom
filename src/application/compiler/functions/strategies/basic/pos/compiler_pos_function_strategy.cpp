#include "compiler_pos_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "lexeme.h"

int PosCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0 && parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "POS") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  (void)result;

  // ld hl, (0xF661)  ; TTYPOS
  cpu.addLdHLii(0xF661);
  // ld h, 0
  cpu.addLdH(0x00);

  return Lexeme::subtype_numeric;
}
