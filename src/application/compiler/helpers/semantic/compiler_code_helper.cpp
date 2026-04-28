/***
 * @file compiler_code_helper.cpp
 * @brief Compiler code helper
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_code_helper.h"

#include "build_options.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "parser.h"

bool CompilerCodeHelper::addCheckTraps() {
  auto& cpu = *context->cpu;
  auto& parser = *context->parser;
  if (parser.getHasTraps()) {
    // call 0x6c25   ; xbasic check traps
    cpu.addCall(def_XBASIC_TRAP_CHECK);
  }

  return context->parser->getHasTraps();
}

void CompilerCodeHelper::addEnableBasicSlot() {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  auto& fix = *context->fixupResolver;
  cpu.addExAF();
  cpu.addExx();
  if (opts.megaROM) {
    // ld a, (EXPTBL)
    cpu.addLdAii(def_EXPTBL);
    // ld h,040h        ; <--- enable jump to here
    cpu.addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    cpu.addCall(def_ENASLT);
    // ei
    cpu.addEI();
  } else {
    // call enable basic slot function
    if (context->enableRomBasicMark) {
      fix.addFix(context->enableRomBasicMark->symbol);
    } else {
      context->enableRomBasicMark = fix.addMark();
    }
    cpu.addCall(0x0000);
  }
  cpu.addExx();
  cpu.addExAF();
}

void CompilerCodeHelper::addCallBDOS() {
  auto& cpu = *context->cpu;

  // call BDOS
  cpu.addCall(def_DSKBAS);
}

void CompilerCodeHelper::addCallBDOSWE() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  shared_ptr<FixNode> errorMark;
  shared_ptr<FixNode> abortMark;
  shared_ptr<FixNode> doneMark;

  // set BDOS error/abort handler addresses
  cpu.addPushHL();
  cpu.addLdHLii(0xF323);  // save error handler address
  cpu.addLdiiHL(def_ARG);
  cpu.addLdHLii(0xF1E6);  // save abort handler address
  cpu.addLdiiHL(def_ARG + 2);
  errorMark = fixup.addMark();
  cpu.addLdHL(0x0000);
  cpu.addLdiiHL(def_ARG + 4);
  cpu.addLdHL(def_ARG + 4);
  cpu.addLdiiHL(0xF323);  // set error handler address to errorMark
  abortMark = fixup.addMark();
  cpu.addLdHL(0x0000);
  cpu.addLdiiHL(0xF1E6);  // set abort handler address to abortMark
  cpu.addPopHL();

  // save stack pointer
  cpu.addLdiiSP(def_DAC);

  // call BDOS
  cpu.addCall(def_DSKBAS);

  // jump to done if successful
  doneMark = fixup.addMark();
  cpu.addJp(0x0000);

  // error handler
  errorMark->aimHere();
  cpu.addLdAC();  // get error code
  cpu.addLdC(2);  // reply = abort
  cpu.addRet();

  // abort handler
  abortMark->aimHere();
  cpu.addAnd(0x7F);        // clear bit 7
  cpu.addNeg();            // turn error code value to negative (A = -A)
  cpu.addLdSPii(def_DAC);  // restore stack pointer
  addDisableBasicSlot();   // restore page 1 to kernel slot

  doneMark->aimHere();

  // restore default BDOS error/abort handler addresses
  cpu.addPushHL();
  cpu.addLdHLii(def_ARG);
  cpu.addLdiiHL(0xF323);  // error handler address
  cpu.addLdHLii(def_ARG + 2);
  cpu.addLdiiHL(0xF1E6);  // abort handler address
  cpu.addPopHL();
}

void CompilerCodeHelper::addDisableBasicSlot() {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  auto& fix = *context->fixupResolver;
  cpu.addExAF();
  cpu.addExx();
  if (opts.megaROM) {
    // ld a, (SLTSTR)
    cpu.addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    cpu.addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    cpu.addCall(def_ENASLT);
    // ei
    cpu.addEI();
  } else {
    // call disable basic slot function
    if (context->disableRomBasicMark)
      fix.addFix(context->disableRomBasicMark->symbol);
    else
      context->disableRomBasicMark = fix.addMark();
    cpu.addCall(0x0000);
  }
  cpu.addExx();
  cpu.addExAF();
}

void CompilerCodeHelper::beginBasicSetStmt(string name) {
  auto& cpu = *context->cpu;
  int i, l = name.size();
  char* s = (char*)name.c_str();

  // ld hl, BUF
  cpu.addLdHL(def_BUF);
  // push hl
  cpu.addPushHL();

  // ; set subcommand
  for (i = 0; i < l; i++) {
    addBasicChar(s[i]);
  }
}

void CompilerCodeHelper::endBasicSetStmt() {
  auto& cpu = *context->cpu;
  //   xor a
  cpu.addXorA();
  //   ld (hl), a
  cpu.addLdiHLA();
  //   inc hl
  cpu.addIncHL();
  //   ld (hl), a
  cpu.addLdiHLA();

  // pop hl
  cpu.addPopHL();

  // ld a, (VERSION)
  cpu.addLdAii(def_VERSION);
  // and a
  cpu.addAndA();
  // jr z, skip
  cpu.addJrZ(9);

  //   ld a, (hl)      ; first character
  cpu.addLdAiHL();
  //   ld ix, (SET)    ; SET
  cpu.addLdIXii(def_SET_STMT);
  //   call CALBAS
  cpu.addCall(def_CALBAS);
  //   ei
  cpu.addEI();

  // skip:
}

void CompilerCodeHelper::addBasicChar(char c) {
  auto& cpu = *context->cpu;
  // ld (hl), char
  cpu.addLdiHL(c);
  // inc hl
  cpu.addIncHL();
}

CompilerCodeHelper::CompilerCodeHelper(shared_ptr<CompilerContext> context)
    : context(context) {}

CompilerCodeHelper::~CompilerCodeHelper() = default;
