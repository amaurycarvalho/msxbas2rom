#include "compiler_fpos_function_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

int FposCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "FPOS") return Lexeme::subtype_unknown;

  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] != Lexeme::subtype_numeric) return Lexeme::subtype_unknown;

  context->file_support = true;

  // hl = file number
  // call cmd_fpos
  context->codeOptimizer->addKernelCall(DISP_cmd_fpos);

  return Lexeme::subtype_numeric;
}
