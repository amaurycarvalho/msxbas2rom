#include "compiler_val_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int ValCompilerFunctionStrategy::execute(CompilerContext* context,
                                         ActionNode* action,
                                         int* result,
                                         unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "VAL") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  if (result[0] == Lexeme::subtype_numeric) {
    // its ok, return same parameter value
    return result[0];

  } else if (result[0] == Lexeme::subtype_single_decimal ||
             result[0] == Lexeme::subtype_double_decimal) {
    // its ok, return same parameter value
    return result[0];

  } else if (result[0] == Lexeme::subtype_string) {
    // call 0x7e07   ; VAL function - xbasic string to float (in hl, out b:hl)
    cpu.addCall(def_XBASIC_VAL);
    result[0] = Lexeme::subtype_single_decimal;

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
