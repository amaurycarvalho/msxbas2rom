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
    if (context->enable_basic_mark) {
      fix.addFix(context->enable_basic_mark->symbol);
    } else {
      context->enable_basic_mark = fix.addMark();
    }
    cpu.addCall(0x0000);
  }
  cpu.addExx();
  basicOnPage1 = true;
}

void CompilerCodeHelper::addCallBDOS() {
  auto& cpu = *context->cpu;
  bool basicOnPage1ManualSet = false;

  if (!basicOnPage1) {
    addEnableBasicSlot();
    basicOnPage1ManualSet = true;
  }

  // call BDOS
  cpu.addCall(def_DSKBAS);

  if (basicOnPage1ManualSet) {
    addDisableBasicSlot();
  }
}

void CompilerCodeHelper::addCallBDOSWE() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  shared_ptr<FixNode> errorMark;
  shared_ptr<FixNode> abortMark;
  shared_ptr<FixNode> doneMark;
  bool basicOnPage1ManualSet = false;

  if (!basicOnPage1) {
    addEnableBasicSlot();
    basicOnPage1ManualSet = true;
  }

  cpu.addPushHL();

  // save BDOS error handler addresses
  cpu.addLdHLii(0xF323);
  cpu.addLdiiHL(def_DSKERRBAK);

  // set BDOS error handler address
  errorMark = fixup.addMark();
  cpu.addLdHL(0x0000);
  cpu.addLdiiHL(def_SPADDRBAK);
  cpu.addLdHL(def_SPADDRBAK);
  cpu.addLdiiHL(0xF323);

  // set BDOS abort handler address
  abortMark = fixup.addMark();
  cpu.addLdHL(0x0000);
  cpu.addLdiiHL(0xF1E6);

  // call BDOS
  cpu.addPopHL();
  cpu.addCall(def_DSKBAS);

  // jump to done if successful
  doneMark = fixup.addMark();
  cpu.addJp(0x0000);

  // error handler
  errorMark->symbol->address = cpu.context->code_pointer;
  cpu.addLdAC();  // copy error code to A
  cpu.addLdC(2);  // response = abort
  cpu.addRet();

  abortMark->symbol->address = cpu.context->code_pointer;
  cpu.addXorA();
  cpu.addIncA();

  doneMark->symbol->address = cpu.context->code_pointer;

  // restore BDOS error and abort handler addresses
  cpu.addPushHL();
  cpu.addLdHLii(def_DSKERRBAK);
  cpu.addLdiiHL(0xF323);
  cpu.addLdHL(0x0000);
  cpu.addLdiiHL(0xF1E6);
  cpu.addPopHL();

  if (basicOnPage1ManualSet) {
    addDisableBasicSlot();
  }
}

void CompilerCodeHelper::addDisableBasicSlot() {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  auto& fix = *context->fixupResolver;
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
    if (context->disable_basic_mark)
      fix.addFix(context->disable_basic_mark->symbol);
    else
      context->disable_basic_mark = fix.addMark();
    cpu.addCall(0x0000);
  }
  cpu.addExx();
  basicOnPage1 = false;
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
    : context(context) {
  basicOnPage1 = false;
}

CompilerCodeHelper::~CompilerCodeHelper() = default;
