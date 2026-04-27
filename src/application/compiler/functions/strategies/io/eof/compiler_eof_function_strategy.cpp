#include "compiler_eof_function_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

int EofCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;

  if (action->lexeme->value != "EOF") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& codeHelper = *context->codeHelper;
  auto& expression = *context->expressionEvaluator;
  auto& fixup = *context->fixupResolver;
  shared_ptr<FixNode> eofDiskUnavailable;
  shared_ptr<FixNode> eofDone;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    context->file_support = true;
    // ld (DAC+2), hl
    cpu.addLdiiHL(def_DAC + 2);

    codeHelper.addEnableBasicSlot();

    // ld a, 0                ; drive A:
    cpu.addLdA(0x00);
    // call preflight disk
    cpu.addCall(def_cmd_preflight_disk);
    // and a
    cpu.addAndA();
    // jp nz, eof unavailable
    eofDiskUnavailable = fixup.addMark();
    cpu.addJpNZ(0x0000);

    // call HSAVD          ; alloc disk
    cpu.addCall(0xFE94);
    // call GETIOBLK       ; get io channel control block from DAC
    cpu.addCall(0x6A6A);
    // jr z, $+6           ; file not open
    cpu.addJrZ(0x05);
    // jr c, $+4           ; not a disk drive device
    cpu.addJrC(0x03);
    // call HEOF           ; put in DAC end of file status
    cpu.addCall(0xFEA3);

    codeHelper.addDisableBasicSlot();

    // ld hl, (DAC+2)
    cpu.addLdHLii(def_DAC + 2);
    eofDone = fixup.addMark();
    cpu.addJp(0x0000);

    eofDiskUnavailable->symbol->address = cpu.context->code_pointer;
    codeHelper.addDisableBasicSlot();
    // ld hl, 1            ; treat unavailable disk as eof
    cpu.addLdHL(0x0001);
    eofDone->symbol->address = cpu.context->code_pointer;

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
