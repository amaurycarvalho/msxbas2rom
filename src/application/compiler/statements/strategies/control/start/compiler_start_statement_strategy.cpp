#include "compiler_start_statement_strategy.h"

#include "build_options.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"

void CompilerStartStatementStrategy::cmd_start(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& parser = *context->parser;
  auto& opts = *context->opts;

  // ld (SAVSTK), sp
  cpu.addLdiiSP(0xf6b1);

  // ld a, (SLTSTR)     ; start slot
  cpu.addLdAii(def_SLTSTR);

  // ld h, 0x40
  cpu.addLdH(0x40);

  // call ENASLT        ; enable xbasic page
  cpu.addCall(def_ENASLT);

  // ld hl, HEAP START ADDRESS
  fixup.addFix(context->heap_mark.get());
  cpu.addLdHL(0x0000);

  // ld de, TEMPORARY STRING START ADDRESS
  fixup.addFix(context->temp_str_mark.get());
  cpu.addLdDE(0x0000);

  if (parser.getHasFont()) {
    // ld ix, FONT BUFFER START ADDRESS
    cpu.addByte(0xDD);
    fixup.addFix(context->heap_mark.get())->step = -def_RAM_BUFSIZ;
    cpu.addLdHL(0x0000);
  } else {
    // push hl
    cpu.addPushHL();
    // pop ix
    cpu.addPopIX();
  }

  // call XBASIC INIT                  ; hl=heap start address, de=temporary
  // string start address, bc=data address, ix=font address, a=data segment
  cpu.addCall(def_XBASIC_INIT);

  if (parser.getHasTraps()) {
    if (opts.megaROM) {
      // ld a, 0xFF
      cpu.addLdA(0xFF);
    } else {
      // xor a
      cpu.addXorA();
    }
    // ld (0xFC82), a          ; start of TRPTBL reserved area (megaROM flag to
    // traps)
    cpu.addLdiiA(def_MR_TRAP_FLAG);
  }

  if (opts.megaROM) {
    // ld a, 2
    cpu.addLdA(0x02);
    // call MR_CHANGE_SGM
    cpu.addCall(def_MR_CHANGE_SGM);
  }

  if (parser.getHasData() || parser.getHasIData()) {
    // ld hl, data resource number
    cpu.addLdHL(parser.getResourceCount());
    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);
    // call cmd_restore
    cpu.addCall(def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement
  }

  if (parser.getHasAkm()) {
    // initialize akm player
    cpu.addCall(def_player_initialize);
  }

  // ei
  cpu.addEI();
}

bool CompilerStartStatementStrategy::execute(CompilerContext* context) {
  cmd_start(context);
  return context->compiled;
}
