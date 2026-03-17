#include "compiler_let_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"

void CompilerLetStatementStrategy::cmd_let(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  ActionNode *action, *lex_action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype, result[3];

  if (t != 2) {
    context->syntaxError("Invalid LET parameters count");
    return;
  }

  lex_action = context->current_action->actions[0];
  lexeme = lex_action->lexeme;

  // ld hl, data parameter

  action = context->current_action->actions[1];
  result_subtype = expression.evalExpression(action);

  // do assignment

  if (lexeme->value == "MID$") {
    // cast
    expression.addCast(result_subtype, Lexeme::subtype_string);

    // push hl
    cpu.addPushHL();

    t = lex_action->actions.size();

    if (t < 2 || t > 3) {
      context->syntaxError("Invalid MID$ assignment parameters count");
      return;
    }

    if (!expression.evalOperatorParms(lex_action, t)) {
      context->syntaxError("Invalid MID$ assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;
    result[1] = lex_action->actions[1]->subtype;

    if (t == 2) {
      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        expression.addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      if (result[1] == Lexeme::subtype_string &&
          result[0] == Lexeme::subtype_numeric) {
        // ld a, l         ; start char
        cpu.addLdAL();
        // pop de          ; de=destination string
        cpu.addPopDE();
        // pop hl          ; hl=source string
        cpu.addPopHL();

        // ld b, (hl)      ; number of chars (all from source)
        cpu.addLdBiHL();

        // call 0x7dd8    ; mid assignment (in: hl=source string, b=size,
        // a=start, de=destination string)
        cpu.addCall(def_XBASIC_MID_ASSIGN);

        return;
      }

    } else {
      result[2] = lex_action->actions[2]->subtype;

      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        expression.addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      // ld b, l             ; number of chars
      cpu.addLdBL();

      if (result[1] == Lexeme::subtype_single_decimal ||
          result[1] == Lexeme::subtype_double_decimal) {
        // ld a, b
        cpu.addLdAB();
        // pop bc
        cpu.addPopBC();
        // pop hl
        cpu.addPopHL();
        // push af
        cpu.addPushAF();
        // cast
        expression.addCast(result[1], Lexeme::subtype_numeric);
        // pop bc
        cpu.addPopBC();
        result[1] = Lexeme::subtype_numeric;
      } else {
        // pop hl
        cpu.addPopHL();
      }

      // ld a, l         ; start char
      cpu.addLdAL();

      if (result[2] == Lexeme::subtype_string &&
          result[1] == Lexeme::subtype_numeric &&
          result[0] == Lexeme::subtype_numeric) {
        // pop de          ; de=destination string
        cpu.addPopDE();
        // pop hl          ; hl=source string
        cpu.addPopHL();

        // call 0x7dd8    ; mid assignment (in: hl=source string, b=size,
        // a=start, de=destination string)
        cpu.addCall(def_XBASIC_MID_ASSIGN);

        return;
      }
    }

    context->syntaxError("Invalid MID$ assignment type");

  } else if (lexeme->value == "VDP") {
    // cast
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl          ; data
    cpu.addPushHL();

    t = lex_action->actions.size();

    if (t != 1) {
      context->syntaxError("Invalid VDP assignment parameters count");
      return;
    }

    if (!expression.evalOperatorParms(lex_action, t)) {
      context->syntaxError("Invalid VDP assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;

    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;

    // pop bc           ; data
    cpu.addPopBC();

    // call VDP.set     ; VDP assignment (in: bc=data, hl=register)
    cpu.addCall(def_vdp_set);

  } else if (lexeme->value == "SPRITE$") {
    // cast
    expression.addCast(result_subtype, Lexeme::subtype_string);

    // push hl
    cpu.addPushHL();

    t = lex_action->actions.size();

    if (t != 1) {
      context->syntaxError("Invalid SPRITE$ assignment parameters count");
      return;
    }

    if (!expression.evalOperatorParms(lex_action, t)) {
      context->syntaxError("Invalid SPRITE$ assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;

    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;

    // ld a, l
    cpu.addLdAL();
    // pop hl
    cpu.addPopHL();

    // call 0x7143     ; xbasic SPRITE assignment (in: a=sprite pattern,
    // hl=string)
    cpu.addCall(def_XBASIC_SPRITE_ASSIGN);

  } else {
    // cast

    expression.addCast(result_subtype, lexeme->subtype);

    // do assignment

    context->variableEmitter->addAssignment(lex_action);
  }
}

bool CompilerLetStatementStrategy::execute(CompilerContext* context) {
  cmd_let(context);
  return context->compiled;
}
