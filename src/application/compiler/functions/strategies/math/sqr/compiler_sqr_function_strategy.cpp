#include "compiler_sqr_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int SqrCompilerFunctionStrategy::execute(CompilerContext* context,
                                         ActionNode* action,
                                         int* result,
                                         unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "SQR") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_numeric) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_single_decimal);
    result[0] = Lexeme::subtype_single_decimal;
  }

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // call 0x7ab5         ; xbasic SQR (in b:hl, out b:hl)
    cpu.addCall(def_XBASIC_SQR);

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
