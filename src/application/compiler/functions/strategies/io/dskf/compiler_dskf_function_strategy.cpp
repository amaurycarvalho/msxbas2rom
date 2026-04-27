#include "compiler_dskf_function_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

int DskfCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "DSKF") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& codeHelper = *context->codeHelper;
  auto& expression = *context->expressionEvaluator;
  auto& fixup = *context->fixupResolver;
  shared_ptr<FixNode> invalidParameterErrorMark;
  shared_ptr<FixNode> driveNotFoundErrorMark;
  shared_ptr<FixNode> diskReadingErrorMark;
  shared_ptr<FixNode> doneMark;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] != Lexeme::subtype_numeric) return Lexeme::subtype_unknown;

  context->file_support = true;

  // ---> check parameter
  // in: hl = drive index (0=A:, 1=B:, ...)
  // ld a, l
  cpu.addLdAL();
  cpu.addCp(9);
  // invalid disk parameter
  // jp error if A >= 9
  invalidParameterErrorMark = fixup.addMark();
  cpu.addJpNC(0x0000);

  // ---> check disk
  // call preflight disk ; out: a=0 available, a=1 unavailable
  cpu.addCall(def_cmd_preflight_disk);
  // and a
  cpu.addAndA();
  // jp error if A != 0
  diskReadingErrorMark = fixup.addMark();
  cpu.addJpNZ(0x0000);

  // ---> enable ROM BASIC on page 1
  codeHelper.addEnableBasicSlot();

  // ---> check drivers
  // ld c, 0x18    ; BDOS GetLoginVector (return drivers flag in L)
  cpu.addLdC(0x18);
  cpu.addPushHL();
  codeHelper.addCallBDOS();
  cpu.addPopDE();
  cpu.addLdAL();
  cpu.addAndA();
  // jp error if A = 0
  driveNotFoundErrorMark = fixup.addMark();
  cpu.addJpZ(0x0000);

  // ---> set disk transfer area address
  // ld c, 0x1A    ; BDOS SetDta
  cpu.addLdC(0x1A);
  cpu.addPushDE();
  cpu.addLdDE(def_HEAPSTR);
  codeHelper.addCallBDOS();
  cpu.addPopDE();

  // ---> get disk information
  // ld c, 0x1B    ; BDOS GetAllocationInfo
  cpu.addLdC(0x1B);
  // ld e, drive_number       ; drive number
  codeHelper.addCallBDOSWE();

  // A = sectors per cluster (0xFF if error), HL = free clusters
  // cp 0xFF
  cpu.addCp(0xFF);
  // jp nz, done
  doneMark = fixup.addMark();
  cpu.addJpNZ(0x0000);

  // ---> disk error fallback
  driveNotFoundErrorMark->symbol->address = cpu.context->code_pointer;
  diskReadingErrorMark->symbol->address = cpu.context->code_pointer;
  invalidParameterErrorMark->symbol->address = cpu.context->code_pointer;
  // ld hl, -1
  cpu.addLdHL(0xFFFF);

  doneMark->symbol->address = cpu.context->code_pointer;

  // ---> disable ROM BASIC on page 1
  codeHelper.addDisableBasicSlot();

  return Lexeme::subtype_numeric;
}
