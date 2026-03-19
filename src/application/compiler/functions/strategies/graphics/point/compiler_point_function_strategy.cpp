#include "compiler_point_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int PointCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 2) return Lexeme::subtype_unknown;

  if (action->lexeme->value != "POINT") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[1] == Lexeme::subtype_single_decimal ||
      result[1] == Lexeme::subtype_double_decimal) {
    // ex de,hl
    optimizer.addByteOptimized(0xEB);
    // pop bc
    cpu.addPopBC();
    // pop hl
    cpu.addPopHL();
    // push de
    cpu.addPushDE();

    // cast
    expression.addCast(result[1], Lexeme::subtype_numeric);
    result[1] = Lexeme::subtype_numeric;

    // pop de
    cpu.addPopDE();
    // ex de,hl
    cpu.addExDEHL();

  } else {
    // pop de
    cpu.addPopDE();
  }

  if (result[1] == Lexeme::subtype_numeric &&
      result[0] == Lexeme::subtype_numeric) {
    // call 0x6fa7     ; xbasic POINT (in: de=x, hl=y; out: hl=color)
    cpu.addCall(def_XBASIC_POINT);

    return Lexeme::subtype_numeric;
  }

  return Lexeme::subtype_unknown;
}
