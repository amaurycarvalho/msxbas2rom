#include "compiler_open_statement_strategy.h"

#include "action_node.h"
#include "build_options.h"
#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerOpenStatementStrategy::cmd_open(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  auto& fixup = *context->fixupResolver;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype, state = 0;
  shared_ptr<FixNode> mark;
  bool has[4];

  for (i = 0; i < 4; i++) has[i] = false;

  context->file_support = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      switch (state) {
        // file name
        case 0: {
          has[state] = true;
          state = 1;
          result_subtype = expression.evalExpression(action);
          expression.addCast(result_subtype, Lexeme::subtype_string);

          // push hl
          cpu.addPushHL();
        } break;

        // FOR
        case 1: {
          has[state] = true;
          state = 2;
          if (lexeme->value == "OUT" || lexeme->value == "OUTPUT") {
            // ld a, 2     ; output mode
            cpu.addLdA(0x02);
          } else if (lexeme->value == "APP" || lexeme->value == "APPEND") {
            // ld a, 8     ; append mode
            cpu.addLdA(0x08);
          } else if (lexeme->value == "INPUT") {
            // ld a, 1     ; input mode
            cpu.addLdA(0x01);
          } else {
            // ld a, 4     ; random mode
            cpu.addLdA(0x04);
            i--;
          }
          // push af
          cpu.addPushAF();
        } break;

        // AS
        case 2: {
          has[state] = true;
          state = 3;
          result_subtype = expression.evalExpression(action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // push hl
          cpu.addPushHL();
        } break;

        // LEN
        case 3: {
          has[state] = true;
          state = 4;

          result_subtype = expression.evalExpression(action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
        } break;
      }
    }

    codeHelper.addEnableBasicSlot();

    // LEN
    if (!has[3]) {
      // ld hl, 256  ; default record size
      cpu.addLdHL(0x0100);
    }
    // ld (RECSIZ), hl
    cpu.addLdiiHL(def_RECSIZ);

    // AS
    if (has[2]) {
      // pop hl
      cpu.addPopHL();
      // ld a, l
      cpu.addLdAL();
    } else {
      // xor a
      cpu.addXorA();
    }
    // ld (TEMP), a
    cpu.addLdiiA(def_TEMP);

    // FOR
    if (has[1]) {
      // pop af
      cpu.addPopAF();
    } else {
      // xor a
      cpu.addXorA();
    }
    // ld (TEMP+1), a           ; file mode
    cpu.addLdiiA(def_TEMP + 1);

    if (opts.megaROM) {
      // ld hl, 0x0000             ; get return point address
      mark = fixup.addMark();
      optimizer.addLdHLmegarom();
      // ld c, l
      cpu.addLdCL();
      // ld b, h
      cpu.addLdBH();
    } else {
      // ld bc, 0x0000             ; get return point address
      mark = fixup.addMark();
      cpu.addLdBC(0x0000);
    }

    // FILE
    if (has[0]) {
      // pop hl
      cpu.addPopHL();
    } else {
      // ld hl, NULL STR
      cpu.addLdHL(0x8000);
    }

    // push bc            ; save return point to run after FILESPEC
    cpu.addPushBC();

    // ld e, (hl)
    cpu.addLdEiHL();

    // inc hl
    cpu.addIncHL();

    // push hl
    cpu.addPushHL();

    // jp FILESPEC        ; in e=string size, hl=string start; out d = device
    // code, FILNAM
    cpu.addJp(def_FILESPEC);

    mark->symbol->address =
        cpu.context->code_pointer;  // return point after FILESPEC

    // ld a, (TEMP+1)
    cpu.addLdAii(def_TEMP + 1);
    // ld e, a                ; file mode
    cpu.addLdEA();

    // ld a, 0                ; drive A:
    cpu.addLdA(0x00);
    // call preflight disk
    cpu.addCall(def_cmd_preflight_disk);
    // and a
    cpu.addAndA();
    // jp nz, skip OPEN
    auto skipOpenMark = fixup.addMark();
    cpu.addJpNZ(0x0000);

    // ld a, (TEMP)           ; io number
    cpu.addLdAii(def_TEMP);

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // call OPEN     ; in: a = i/o number, e = filemode, d = devicecode
    cpu.addCall(def_OPEN);
    skipOpenMark->symbol->address = cpu.context->code_pointer;

    codeHelper.addDisableBasicSlot();

  } else {
    context->syntaxError("Empty OPEN statement");
  }
}

bool CompilerOpenStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_open(context);
  return context->compiled;
}
