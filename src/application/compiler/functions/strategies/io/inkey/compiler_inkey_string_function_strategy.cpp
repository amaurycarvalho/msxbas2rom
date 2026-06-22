#include "compiler_inkey_string_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int InkeyStringCompilerFunctionStrategy::execute(
    shared_ptr<CompilerContext> context, shared_ptr<ActionNode> action,
    int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "INKEY$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;
  auto& variable = *context->variableEmitter;

  // call 0x009C        ; CHSNS
  cpu.addCall(def_CHSNS);
  // ld hl, temporary string
  variable.addTempStr(true);
  // call 0x7e5e   ; xbasic INKEY$ (in: hl=dest; out: hl=result)
  optimizer.addKernelCall(DISP_XBASIC_INKEY);

  return Lexeme::subtype_string;
}
