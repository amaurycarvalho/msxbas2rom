#include "compiler_key_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerKeyStatementStrategy::cmd_key(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action1 = context->current_action->actions[0];
    next_lexeme = action1->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x00CF   ; DSPFNK - (0xF3DE = CNSDFG: function keys
      // presentation)
      cpu.addCall(0x00CF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x00CC   ; ERAFNK
      cpu.addCall(0x00CC);
    } else {
      context->syntaxError("Invalid KEY statement");
    }

  } else if (t == 2) {
    action2 = context->current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, key number
    action1 = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action1);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    cpu.addDecHL();

    if ((next_lexeme->type == Lexeme::type_keyword &&
         next_lexeme->subtype == Lexeme::subtype_function) ||
        (next_lexeme->type == Lexeme::type_operator &&
         next_lexeme->value == "+") ||
        (next_lexeme->subtype == Lexeme::subtype_string)) {
      // add hl, hl
      cpu.addAddHLHL();
      // add hl, hl
      cpu.addAddHLHL();
      // add hl, hl
      cpu.addAddHLHL();
      // add hl, hl      ; key number * 16
      cpu.addAddHLHL();
      // ld de, 0xF87F   ; FNKSTR (10 x 16 bytes)
      cpu.addLdDE(0xf87f);
      // add hl, de
      cpu.addAddHLDE();

      // push hl
      cpu.addPushHL();
      //   ld a, 32        ; space
      cpu.addLdA(0x20);
      //   ld (hl), a
      cpu.addLdiHLA();
      //   ld e, l
      cpu.addLdEL();
      //   ld d, h
      cpu.addLdDH();
      //   inc de
      cpu.addIncDE();
      //   ld bc, 15
      cpu.addLdBC(0x000F);
      //   ldir
      cpu.addLDIR();

      //   ld hl, variable address
      result_subtype = expression.evalExpression(action2);
      expression.addCast(result_subtype, Lexeme::subtype_string);

      //   xor a
      cpu.addXorA();
      //   ld c, (hl)
      cpu.addLdCiHL();
      //   ld b, a
      cpu.addLdBA();
      //   inc hl
      cpu.addIncHL();

      // pop de
      cpu.addPopDE();

      // or c
      cpu.addOrC();
      // jr z, $+3
      cpu.addJrZ(0x02);
      //   ldir
      cpu.addLDIR();
      // ld (de), a
      cpu.addLdiDEA();
      // call 0x00C9    ; FNKSB
      cpu.addCall(0x00C9);

    } else {
      // ld e, l
      cpu.addLdEL();
      // ld d, h
      cpu.addLdDH();
      // add hl, de
      cpu.addAddHLDE();
      // add hl, de       ; key number * 3
      cpu.addAddHLDE();

      // ld de, 0xFC4C    ; KEY state position = key number * 3 + 0xFC4C
      cpu.addLdDE(0xfc4c);

      // add hl, de
      cpu.addAddHLDE();

      if (next_lexeme->type == Lexeme::type_keyword &&
          next_lexeme->value == "ON") {
        // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
        cpu.addCall(def_XBASIC_TRAP_ON);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "OFF") {
        // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
        cpu.addCall(def_XBASIC_TRAP_OFF);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "STOP") {
        // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
        cpu.addCall(def_XBASIC_TRAP_STOP);
      } else {
        context->syntaxError("Invalid KEY statement");
      }
    }

  } else {
    context->syntaxError("Empty KEY statement");
  }
}

bool CompilerKeyStatementStrategy::execute(CompilerContext* context) {
  cmd_key(context);
  return context->compiled;
}
