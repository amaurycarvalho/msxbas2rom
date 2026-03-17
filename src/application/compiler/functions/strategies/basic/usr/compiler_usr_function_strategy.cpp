#include "compiler_usr_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int UsrCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;

  const string& keyword = action->lexeme->value;
  if (keyword != "USR" && keyword != "USR0" && keyword != "USR1" &&
      keyword != "USR2" && keyword != "USR3" && keyword != "USR4" &&
      keyword != "USR5" && keyword != "USR6" && keyword != "USR7" &&
      keyword != "USR8" && keyword != "USR9") {
    return Lexeme::subtype_unknown;
  }

  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  // cast
  expression.addCast(result[0], Lexeme::subtype_numeric);
  result[0] = Lexeme::subtype_numeric;

  if (context->has_defusr) {
    if (keyword == "USR" || keyword == "USR0") {
      // xor a
      cpu.addXorA();
    } else if (keyword == "USR1") {
      // ld a, 1
      cpu.addLdA(0x01);
    } else if (keyword == "USR2") {
      // ld a, 2
      cpu.addLdA(0x02);
    } else if (keyword == "USR3") {
      // ld a, 3
      cpu.addLdA(0x03);
    } else if (keyword == "USR4") {
      // ld a, 4
      cpu.addLdA(0x04);
    } else if (keyword == "USR5") {
      // ld a, 5
      cpu.addLdA(0x05);
    } else if (keyword == "USR6") {
      // ld a, 6
      cpu.addLdA(0x06);
    } else if (keyword == "USR7") {
      // ld a, 7
      cpu.addLdA(0x07);
    } else if (keyword == "USR8") {
      // ld a, 8
      cpu.addLdA(0x08);
    } else if (keyword == "USR9") {
      // ld a, 9
      cpu.addLdA(0x09);
    }

    // call XBASIC_USR
    cpu.addCall(def_XBASIC_USR);
  } else {
    if (keyword == "USR" || keyword == "USR0") {
      // call usr0
      cpu.addCall(def_usr0);
    } else if (keyword == "USR1") {
      // call usr1
      cpu.addCall(def_usr1);
    } else if (keyword == "USR2") {
      // call usr2
      cpu.addCall(def_usr2);
    } else if (keyword == "USR3") {
      // call usr3
      cpu.addCall(def_usr3);
    }
  }

  return result[0];
}
