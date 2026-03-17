#include "compiler_color_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerColorStatementStrategy::cmd_color(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, subaction;
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "NEW") {
        // ld a, (VERSION)
        cpu.addLdAii(def_VERSION);
        // and a
        cpu.addAndA();
        // jr z, skip
        cpu.addJrZ(8);
        //   ld ix, 0x0141
        cpu.addLdIX(0x0141);
        //   call EXTROM
        cpu.addCall(def_EXTROM);
        //   ei
        cpu.addEI();
        // skip:

      } else if (lexeme->value == "RESTORE") {
        // ld a, (VERSION)
        cpu.addLdAii(def_VERSION);
        // and a
        cpu.addAndA();
        // jr z, skip
        cpu.addJrZ(8);
        //   ld ix, 0x0145
        cpu.addLdIX(def_RSTPLT);
        //   call EXTROM
        cpu.addCall(def_EXTROM);
        //   ei
        cpu.addEI();
        // skip:

      } else if (lexeme->value == "SPRITE") {
        t = action->actions.size();

        if (t != 2) {
          context->syntaxError("Invalid COLOR SPRITE parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = expression.evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();
          }

          if (i < 1) {
            // push af
            cpu.addPushAF();
          }
        }

        // pop bc
        cpu.addPopBC();
        // call COLOR_SPRITE   ; in: b, a
        cpu.addCall(def_XBASIC_COLOR_SPRITE);

      } else if (lexeme->value == "SPRITE$") {
        t = action->actions.size();

        if (t != 2) {
          context->syntaxError("Invalid COLOR SPRITE$ parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = expression.evalExpression(subaction);

          if (i < 1) {
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();
            // push af
            cpu.addPushAF();
          }
        }

        // pop bc
        cpu.addPopBC();
        // call COLOR_SPRSTR   ; in: b, hl
        cpu.addCall(def_XBASIC_COLOR_SPRSTR);

      } else if (lexeme->value == "RGB") {
        t = action->actions.size();

        if (t < 2 || t > 4) {
          context->syntaxError("Invalid COLOR RGB parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = expression.evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();
          }

          if (i < 3) {
            // push af
            cpu.addPushAF();
          }
        }

        if (t < 4) {
          // xor a
          cpu.addXorA();
        }
        // ld b, a
        cpu.addLdBA();

        if (t < 3) {
          // ld h, a
          cpu.addLdHA();
        } else {
          // pop hl
          cpu.addPopHL();
        }

        // pop de
        cpu.addPopDE();
        // pop af
        cpu.addPopAF();

        // call COLOR_RGB   ; in: a, d, h, b
        cpu.addCall(def_XBASIC_COLOR_RGB);

      } else {
        context->syntaxError("Invalid COLOR parameters");
      }

    } else {
      for (i = 0; i < t; i++) {
        action = context->current_action->actions[i];
        result_subtype = expression.evalExpression(action);

        if (result_subtype == Lexeme::subtype_null) continue;

        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l
        cpu.addLdAL();

        switch (i) {
          case 0: {
            // ld (0xF3E9), a   ; FORCLR
            cpu.addLdiiA(0xF3E9);
            // ld (ATRBYT), a   ; ATRBYT
            cpu.addLdiiA(def_ATRBYT);
          } break;

          case 1: {
            // ld (0xF3EA), a   ; BAKCLR
            cpu.addLdiiA(0xF3EA);
          } break;

          case 2: {
            // ld (0xF3EB), a   ; BDRCLR
            cpu.addLdiiA(0xF3EB);
          } break;

          default: {
            context->syntaxError("COLOR parameters not supported");
            return;
          }
        }
      }

      // ld a, (SCRMOD)
      cpu.addLdAii(def_SCRMOD);

      // call 0x0062   ; CHGCLR
      cpu.addCall(0x0062);
    }

  } else {
    context->syntaxError("COLOR with empty parameters");
  }
}

bool CompilerColorStatementStrategy::execute(CompilerContext* context) {
  cmd_color(context);
  return context->compiled;
}
