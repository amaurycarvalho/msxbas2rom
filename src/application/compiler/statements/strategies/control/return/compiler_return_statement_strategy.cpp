#include "compiler_return_statement_strategy.h"

#include "build_options.h"
#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"

void CompilerReturnStatementStrategy::cmd_return(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& optimizer = *context->codeOptimizer;
  auto& opts = *context->opts;
  shared_ptr<Lexeme> lexeme;
  int t = context->current_action->actions.size();

  if (t == 0) {
    // ret
    cpu.addRet();
    return;

  } else if (t == 1) {
    lexeme = context->current_action->actions[0].get()->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        if (opts.megaROM) {
          // pop bc           ; delete old return segment/address
          cpu.addPopBC();
          // pop de           ; delete old return segment/address
          cpu.addPopDE();
          // ld a, 0x48       ; verify if running on trap (MR_CALL_TRAP)
          cpu.addLdA(0x48);
          // cp b
          cpu.addCpB();
          // jp nz, address   ; if not, jump to new address and segment
          fixup.addFix(lexeme->value);
          cpu.addJpNZ(0x0000);

          // pop de           ; fix trap return control
          cpu.addPopDE();
          // pop hl
          cpu.addPopHL();
          // exx
          cpu.addExx();
          // special ld hl, address
          fixup.addFix(lexeme->value);
          optimizer.addLdHLmegarom();
          // ex (sp), hl      ; new return address
          cpu.addExiSPHL();
          // exx
          cpu.addExx();
          // push hl
          cpu.addPushHL();
          // push de
          cpu.addPushDE();
          // push af          ; new return segment
          cpu.addPushAF();
          // push bc          ; trap return
          cpu.addPushBC();
          // ret
          cpu.addRet();

        } else {
          // pop bc           ; delete old return address
          cpu.addPopBC();
          // ld a, 0x6C       ; verify if running on trap
          cpu.addLdA(0x6C);
          // cp b
          cpu.addCpB();
          // jp nz, address   ; jump to new address
          fixup.addFix(lexeme->value);
          cpu.addJpNZ(0x0000);

          // pop de           ; fix trap return control
          cpu.addPopDE();
          // ld hl, address
          fixup.addFix(lexeme->value);
          cpu.addLdHL(0x0000);
          // ex (sp), hl
          cpu.addExiSPHL();
          // push de
          cpu.addPushDE();
          // push bc
          cpu.addPushBC();
          // ret
          cpu.addRet();
        }

        return;
      }
    }
  }

  context->syntaxError("Invalid RETURN parameters");
}

bool CompilerReturnStatementStrategy::execute(CompilerContext* context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_return(context);
  return context->compiled;
}
