#include "compiler_asc_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "lexeme.h"

int AscCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "ASC") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;

  if (result[0] == Lexeme::subtype_string) {
    // ex de, hl
    optimizer.addByteOptimized(0xEB);
    // ld hl, 0
    cpu.addLdHL(0x0000);
    // ld a, (de)
    cpu.addLdAiDE();
    // and a
    cpu.addAndA();
    // jr z,$+4
    cpu.addJrZ(0x03);
    //   inc de
    cpu.addIncDE();
    //   ld a, (de)
    cpu.addLdAiDE();
    //   ld l, a
    cpu.addLdLA();

    result[0] = Lexeme::subtype_numeric;

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
