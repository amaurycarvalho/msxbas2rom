#include "compiler_string_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_variable_emitter.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int StringCompilerFunctionStrategy::execute(CompilerContext* context,
                                            ActionNode* action,
                                            int* result,
                                            unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 2) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "STRING$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  } else if (result[0] == Lexeme::subtype_string) {
    // inc hl
    cpu.addIncHL();
    // ld l, (hl)
    cpu.addLdLiHL();
    // ld h, 0
    cpu.addLdH(0x00);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[1] == Lexeme::subtype_numeric) {
    // pop de
    cpu.addPopDE();
  } else if (result[1] == Lexeme::subtype_single_decimal ||
             result[1] == Lexeme::subtype_double_decimal) {
    // pop bc
    cpu.addPopAF();
    // pop de
    cpu.addPopDE();
    // push hl
    cpu.addPushHL();
    //   ex de,hl
    cpu.addExDEHL();
    // cast
    expression.addCast(result[1], Lexeme::subtype_numeric);
    //   ex de,hl
    cpu.addExDEHL();
    // pop hl
    cpu.addPopHL();
    result[1] = Lexeme::subtype_numeric;
  } else
    result[1] = Lexeme::subtype_unknown;

  if (result[0] == Lexeme::subtype_numeric &&
      result[1] == Lexeme::subtype_numeric) {
    // ld a, l
    cpu.addLdAL();
    // ld b, e
    cpu.addLdBE();
    // ld hl, temporary string
    variable.addTempStr(true);
    // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars, a=char)
    cpu.addCall(def_XBASIC_STRING);

    return Lexeme::subtype_string;
  }

  return Lexeme::subtype_unknown;
}
