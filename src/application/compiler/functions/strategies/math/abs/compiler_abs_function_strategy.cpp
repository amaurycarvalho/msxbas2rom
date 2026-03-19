#include "compiler_abs_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int AbsCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "ABS") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  if (result[0] == Lexeme::subtype_numeric) {
    // call 0x5b36         ; xbasic ABS (in hl, out hl)
    cpu.addCall(def_XBASIC_ABS_INT);

  } else if (result[0] == Lexeme::subtype_single_decimal ||
             result[0] == Lexeme::subtype_double_decimal) {
    // xbasic ABS (in b:hl, out b:hl)
    // res 7,h
    cpu.addWord(0xCB, 0xBC);

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
