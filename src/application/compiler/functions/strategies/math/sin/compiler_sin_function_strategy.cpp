#include "compiler_sin_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int SinCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "SIN") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_numeric) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_single_decimal);
    result[0] = Lexeme::subtype_single_decimal;
  }

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // call 0x7936         ; xbasic SIN (in b:hl, out b:hl)
    cpu.addCall(def_XBASIC_SIN);

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
