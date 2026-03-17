#include "compiler_heap_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int HeapCompilerFunctionStrategy::execute(CompilerContext* context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "HEAP") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // ld hl, (HEAPSTR)
  cpu.addLdHLii(def_HEAPSTR);

  return Lexeme::subtype_numeric;
}
