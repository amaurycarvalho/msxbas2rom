#include "compiler_sgn_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int SgnCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "SGN") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  if (result[0] == Lexeme::subtype_numeric) {
    // call 0x5b5d         ; xbasic SGN (in hl, out hl)
    cpu.addCall(def_XBASIC_SGN_INT);

  } else if (result[0] == Lexeme::subtype_single_decimal ||
             result[0] == Lexeme::subtype_double_decimal) {
    // call 0x5b72         ; xbasic SGN (in b:hl, out b:hl)
    cpu.addCall(def_XBASIC_SGN_FLOAT);

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
