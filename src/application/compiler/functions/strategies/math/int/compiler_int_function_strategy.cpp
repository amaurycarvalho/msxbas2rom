#include "compiler_int_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int IntCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "INT") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  if (result[0] == Lexeme::subtype_numeric) {
    // its ok, return same parameter value
    return result[0];

  } else if (result[0] == Lexeme::subtype_single_decimal ||
             result[0] == Lexeme::subtype_double_decimal) {
    // call 0x78e5         ; xbasic INT
    cpu.addCall(def_XBASIC_INT);

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
