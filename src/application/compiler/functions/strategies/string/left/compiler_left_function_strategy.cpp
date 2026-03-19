#include "compiler_left_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int LeftCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 2) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "LEFT$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[1] == Lexeme::subtype_string &&
      result[0] == Lexeme::subtype_numeric) {
    // ld a, l
    cpu.addLdAL();
    // pop hl
    cpu.addPopHL();

    // call 0x7d99     ; xbasic left string (in: a=size, hl=source; out: hl=BUF)
    cpu.addCall(def_XBASIC_LEFT);
    // ld de, temporary string
    variable.addTempStr(false);
    // push de
    cpu.addPushDE();
    //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
    //   out: hl end of string)
    cpu.addCall(def_XBASIC_COPY_STRING);
    // pop hl
    cpu.addPopHL();

    return Lexeme::subtype_string;
  }

  return Lexeme::subtype_unknown;
}
