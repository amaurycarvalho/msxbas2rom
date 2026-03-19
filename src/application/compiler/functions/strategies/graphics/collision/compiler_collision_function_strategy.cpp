#include "compiler_collision_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int CollisionCompilerFunctionStrategy::execute(
    shared_ptr<CompilerContext> context, shared_ptr<ActionNode> action,
    int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "COLLISION") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;

  switch (parmCount) {
    case 0: {
      // CALL SUB_SPRCOL_ALL
      cpu.addCall(def_usr3_COLLISION_ALL);
      return Lexeme::subtype_numeric;
    }

    case 1: {
      auto& expression = *context->expressionEvaluator;

      if (result[0] == Lexeme::subtype_unknown) return Lexeme::subtype_unknown;

      // cast
      expression.addCast(result[0], Lexeme::subtype_numeric);
      result[0] = Lexeme::subtype_numeric;

      // call SUB_SPRCOL_ONE
      cpu.addCall(def_usr3_COLLISION_ONE);

      return Lexeme::subtype_numeric;
    }

    case 2: {
      auto& expression = *context->expressionEvaluator;

      if (result[0] == Lexeme::subtype_unknown ||
          result[1] == Lexeme::subtype_unknown) {
        return Lexeme::subtype_unknown;
      }

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
        // call SUB_SPRCOL_COUPLE
        cpu.addCall(def_usr3_COLLISION_COUPLE);
        return Lexeme::subtype_numeric;
      }

      return Lexeme::subtype_unknown;
    }

    default:
      return Lexeme::subtype_unknown;
  }
}
