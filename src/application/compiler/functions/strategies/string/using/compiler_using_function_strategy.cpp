#include "compiler_using_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_float_converter.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int UsingCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 2) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "USING$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;
  auto& floatConverter = *context->floatConverter;

  // Preprocess format string before parameter evaluation.
  if (parmCount >= 2) {
    shared_ptr<ActionNode> next_action = action->actions[1];
    shared_ptr<Lexeme> lexeme2 = next_action ? next_action->lexeme : 0;
    if (lexeme2 && lexeme2->type == Lexeme::type_literal &&
        lexeme2->subtype == Lexeme::subtype_string) {
      int r = floatConverter.getUsingFormat(lexeme2->value);
      lexeme2->subtype = Lexeme::subtype_numeric;
      lexeme2->value = to_string(r);
    }
  }

  if (!expression.evalOperatorParms(action, parmCount)) {
    return Lexeme::subtype_unknown;
  }

  for (unsigned int i = 0; i < parmCount; i++) {
    shared_ptr<ActionNode> next_action = action->actions[i];
    result[i] = next_action->subtype;
    if (result[i] == Lexeme::subtype_unknown) return result[i];
  }

  if (result[0] == Lexeme::subtype_double_decimal ||
      result[0] == Lexeme::subtype_numeric) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_single_decimal);
    result[0] = Lexeme::subtype_single_decimal;
  }

  if (result[1] == Lexeme::subtype_string &&
      result[0] == Lexeme::subtype_single_decimal) {
    // ld c, b
    cpu.addLdCB();
    // ex de, hl
    cpu.addExDEHL();
    // pop hl
    cpu.addPopHL();

    // call XBASIC_USING    ; hl = item format string, c:de = float, out
    // hl=string
    cpu.addCall(def_XBASIC_USING);
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

  } else if (result[1] == Lexeme::subtype_numeric &&
             result[0] == Lexeme::subtype_single_decimal) {
    // pop de
    cpu.addPopDE();
    // push de
    cpu.addPushDE();
    //   ld a, e
    cpu.addLdAE();
    //   rrca
    cpu.addRRCA();
    //   rrca
    cpu.addRRCA();
    //   rrca
    cpu.addRRCA();
    //   rrca
    cpu.addRRCA();
    //   and 0x0F
    cpu.addAnd(0x0F);
    //   ld d, a
    cpu.addLdDA();
    //   ld a, e
    cpu.addLdAE();
    //   and 0x0F
    cpu.addAnd(0x0F);
    //   ld e, a
    cpu.addLdEA();
    // pop af
    cpu.addPopAF();

    // call XBASIC_USING_DO    ; a=format, d=thousand digits, e=decimal digits,
    // b:hl=number, out hl=string
    cpu.addCall(def_XBASIC_USING_DO);
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
