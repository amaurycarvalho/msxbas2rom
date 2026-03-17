#include "compiler_input_string_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int InputStringCompilerFunctionStrategy::execute(CompilerContext* context,
                                                 shared_ptr<ActionNode> action,
                                                 int* result,
                                                 unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;

  if (action->lexeme->value != "INPUT$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    // ex de, hl
    optimizer.addByteOptimized(0xEB);
    // ld hl, temporary string
    variable.addTempStr(true);
    // push hl
    cpu.addPushHL();
    //   ld (hl), e
    cpu.addLdiHLE();
    //   inc hl
    cpu.addIncHL();
    //     call 0x009F        ; CHGET
    cpu.addCall(0x009F);
    //     ld (hl), a
    cpu.addLdiHLA();
    //     inc hl
    cpu.addIncHL();
    //     dec e
    cpu.addDecE();
    //   jr nz,$-8
    cpu.addJrNZ((unsigned char)(0xFF - 7));
    // pop hl
    cpu.addPopHL();

    result[0] = Lexeme::subtype_string;

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
