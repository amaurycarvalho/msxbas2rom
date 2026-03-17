#include "compiler_len_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "lexeme.h"

int LenCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "LEN") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  if (result[0] == Lexeme::subtype_string) {
    // ld l, (hl)
    cpu.addLdLiHL();
    // ld h, 0
    cpu.addLdH(0x00);

    return Lexeme::subtype_numeric;
  }

  return Lexeme::subtype_unknown;
}
