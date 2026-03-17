#include "compiler_ntsc_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int NtscCompilerFunctionStrategy::execute(CompilerContext* context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "NTSC") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // ld hl, 0
  cpu.addLdHL(0x0000);
  // ld a, (NTSC)
  cpu.addLdAii(def_NTSC);
  // and 128   ; bit 7 on?
  cpu.addAnd(0x80);
  // jr nz, $+1
  cpu.addJrNZ(0x01);
  //    dec hl
  cpu.addDecHL();

  return Lexeme::subtype_numeric;
}
