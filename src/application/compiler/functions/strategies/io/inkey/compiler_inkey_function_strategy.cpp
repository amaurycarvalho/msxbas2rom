#include "compiler_inkey_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int InkeyCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "INKEY") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // ld hl, 0
  cpu.addLdHL(0x0000);
  // call 0x009C        ; CHSNS
  cpu.addCall(0x009C);
  // jr z,$+5
  cpu.addJrZ(0x04);
  //   call 0x009F        ; CHGET
  cpu.addCall(0x009F);
  //   ld l, a
  cpu.addLdLA();

  return Lexeme::subtype_numeric;
}
