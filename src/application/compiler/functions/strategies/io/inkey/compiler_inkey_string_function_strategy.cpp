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

  auto& cpu = *context->getCpu();
  auto& optimizer = *context->getCodeOptimizer();
  auto& variable = *context->getVariableEmitter();

  // call 0x009C        ; CHSNS
  cpu.addCall(def_CHSNS);
  // call 0x7e5e   ; xbasic INKEY$ (out: hl=BUF)
  optimizer.addKernelCall(DISP_XBASIC_INKEY);
  // ld de, temporary string
  variable.addTempStr(false);
  // push de
  cpu.addPushDE();
  //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
  //   out: hl end of string)
  optimizer.addKernelCall(DISP_XBASIC_COPY_STRING);
  // pop hl
  cpu.addPopHL();

  return Lexeme::subtype_string;
}
