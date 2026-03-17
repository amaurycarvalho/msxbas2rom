#include "compiler_csrlin_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "lexeme.h"

int CsrlinCompilerFunctionStrategy::execute(CompilerContext* context,
                                            shared_ptr<ActionNode> action,
                                            int* result,
                                            unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0 && parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "CSRLIN") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  (void)result;

  // ld hl, (0xF3DC)  ; CSRY
  cpu.addLdHLii(0xF3DC);
  // ld h, 0
  cpu.addLdH(0x00);

  return Lexeme::subtype_numeric;
}
