#include "compiler_space_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int SpaceCompilerFunctionStrategy::execute(CompilerContext* context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "SPACE$" && action->lexeme->value != "SPC")
    return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    // ld a, 0x20      ; space
    cpu.addLdA(0x20);
    // ld b, l
    cpu.addLdBL();
    // ld hl, temporary string
    variable.addTempStr(true);
    // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars, a=char)
    cpu.addCall(def_XBASIC_STRING);

    return Lexeme::subtype_string;
  }

  return Lexeme::subtype_unknown;
}
