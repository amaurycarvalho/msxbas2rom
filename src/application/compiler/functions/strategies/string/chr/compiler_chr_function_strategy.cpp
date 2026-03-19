#include "compiler_chr_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int ChrCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "CHR$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    // ex de, hl
    optimizer.addByteOptimized(0xEB);
    // ld hl, temporary string
    variable.addTempStr(true);
    // inc d
    cpu.addIncD();
    // ld (hl), d
    cpu.addLdiHLD();
    // inc hl
    cpu.addIncHL();
    // ld (hl), e
    cpu.addLdiHLE();
    // dec hl
    cpu.addDecHL();

    return Lexeme::subtype_string;
  }

  return Lexeme::subtype_unknown;
}
