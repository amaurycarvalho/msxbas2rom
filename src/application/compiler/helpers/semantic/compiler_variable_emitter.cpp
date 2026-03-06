/***
 * @file compiler_variable_emitter.cpp
 * @brief Compiler variable emitter
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_variable_emitter.h"

#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"

bool CompilerVariableEmitter::addVarAddress(ActionNode* action) {
  Lexeme *lexeme, *lexeme1, *lexeme2;
  ActionNode *action1, *action2;
  unsigned int i, t;
  int factor, diff;
  int result_subtype;
  bool first;

  lexeme = action->lexeme;
  t = action->actions.size();

  if (lexeme->isArray) {
    switch (t) {
      case 0: {
        context->syntaxError("Array index is missing");
        return false;
      } break;

      case 1: {
        action1 = action->actions[0];
        lexeme1 = action1->lexeme;

        if (lexeme1->type == Lexeme::type_literal) {
          if (lexeme1->subtype == Lexeme::subtype_numeric ||
              lexeme1->subtype == Lexeme::subtype_single_decimal ||
              lexeme1->subtype == Lexeme::subtype_double_decimal) {
            try {
              i = stoi(lexeme1->value) * lexeme->x_factor;
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     lexeme1->value.c_str());
              i = 0;
            }

            // ld hl, variable_address +  (x_index * x_factor)
            context->fixupResolver->addFix(lexeme)->step = i;
            context->cpu->addLdHL(0x0000);

          } else {
            context->syntaxError("Invalid array index type");
          }

        } else {
          // ld hl, x index
          result_subtype =
              context->expressionEvaluator->evalExpression(action1);
          context->expressionEvaluator->addCast(result_subtype,
                                                Lexeme::subtype_numeric);

          if (lexeme->x_factor == 2) {
            // add hl, hl
            context->cpu->addAddHLHL();
          } else if (lexeme->x_factor == 3) {
            // ld d, h
            context->cpu->addLdDH();
            // ld e, l
            context->cpu->addLdEL();
            // add hl, hl
            context->cpu->addAddHLHL();
            // add hl, de
            context->cpu->addAddHLDE();
          } else if (lexeme->x_factor == 256) {
            // ld h, l
            context->cpu->addLdHL();
            // ld l, 0
            context->cpu->addLdL(0x00);
          } else {
            // ld de, x_factor
            context->cpu->addLdDE(lexeme->x_factor);

            // call 0x761b    ; integer multiplication (hl = hl * de)
            context->cpu->addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

          // ld de, variable
          context->fixupResolver->addFix(lexeme);
          context->cpu->addLdDE(0x0000);

          // add hl, de   ; hl = variable_address +  (x_index * x_factor)
          context->cpu->addAddHLDE();
        }

      } break;

      case 2: {
        action1 = action->actions[1];
        lexeme1 = action1->lexeme;

        action2 = action->actions[0];
        lexeme2 = action2->lexeme;

        if (lexeme1->type == Lexeme::type_literal &&
            lexeme2->type == Lexeme::type_literal) {
          if (lexeme1->subtype == Lexeme::subtype_numeric ||
              lexeme1->subtype == Lexeme::subtype_single_decimal ||
              lexeme1->subtype == Lexeme::subtype_double_decimal) {
            try {
              i = stoi(lexeme1->value) * lexeme->x_factor;
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     lexeme1->value.c_str());
              i = 0;
            }

            if (lexeme2->subtype == Lexeme::subtype_numeric ||
                lexeme2->subtype == Lexeme::subtype_single_decimal ||
                lexeme2->subtype == Lexeme::subtype_double_decimal) {
              try {
                i += stoi(lexeme2->value) * lexeme->y_factor;
              } catch (exception& e) {
                printf("Warning: error while converting numeric constant %s\n",
                       lexeme2->value.c_str());
              }

              // ld hl, variable_address +  (x_index * x_factor) + (y_index *
              // y_factor)
              context->fixupResolver->addFix(lexeme)->step = i;
              context->cpu->addLdHL(0x0000);

            } else {
              context->syntaxError("Invalid array 2nd index type");
            }

          } else {
            context->syntaxError("Invalid array 1st index type");
          }

        } else {
          // ld hl, x index
          result_subtype =
              context->expressionEvaluator->evalExpression(action1);
          context->expressionEvaluator->addCast(result_subtype,
                                                Lexeme::subtype_numeric);

          if (lexeme->x_factor == 2) {
            // add hl, hl
            context->cpu->addAddHLHL();
          } else if (lexeme->x_factor == 3) {
            // ld d, h
            context->cpu->addLdDH();
            // ld e, l
            context->cpu->addLdEL();
            // add hl, hl
            context->cpu->addAddHLHL();
            // add hl, de
            context->cpu->addAddHLDE();
          } else if (lexeme->x_factor == 256) {
            // ld h, l
            context->cpu->addLdHL();
            // ld l, 0
            context->cpu->addLdL(0x00);
          } else {
            // ld de, x_factor
            context->cpu->addLdDE(lexeme->x_factor);

            // call 0x761b    ; integer multiplication (hl = hl * de)
            context->cpu->addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

          // push hl
          context->cpu->addPushHL();

          // ld hl, y index
          result_subtype =
              context->expressionEvaluator->evalExpression(action2);
          context->expressionEvaluator->addCast(result_subtype,
                                                Lexeme::subtype_numeric);

          // calculate y factor * index
          // ---------- old code
          // ld de, y_factor
          // addLdDEnn(lexeme->y_factor);
          // call 0x761b    ; integer multiplication (hl = hl * de)
          // ctx.cpu->addCall(def_XBASIC_MULTIPLY_INTEGERS);
          //--------------------

          factor = lexeme->y_factor;
          if (lexeme->x_factor == 2) {
            factor >>= 1;
          }

          first = true;

          // verify if factor is power of 2
          diff = 1;
          while (factor >= (diff << 1)) {
            diff <<= 1;  // diff *= 2;
          }
          if (diff != factor) {
            // ld b, h           ; save index in bc
            context->cpu->addLdBH();
            // ld c, l
            context->cpu->addLdCL();
          }

          while (factor) {
            if (!first) {
              // ex de, hl     ; save current total
              context->cpu->addExDEHL();
              // ld h, b       ; restore index from bc
              context->cpu->addLdHB();
              // ld l, c
              context->cpu->addLdLC();
            }

            diff = 1;

            while (factor >= (diff << 1)) {
              // add hl, hl    ; x 2
              context->cpu->addAddHLHL();

              diff <<= 1;  // diff *= 2;
            }

            if (first) {
              first = false;
            } else {
              // add hl, de    ; add last total
              context->cpu->addAddHLDE();
            }

            factor -= diff;
          }

          if (lexeme->x_factor == 2) {
            // add hl, hl    ; x 2
            context->cpu->addAddHLHL();
          }

          // pop de
          context->cpu->addPopDE();

          // add hl, de     ; hl = (x_index * x_factor) + (y_index * y_factor)
          context->cpu->addAddHLDE();

          // ld de, variable
          context->fixupResolver->addFix(lexeme);
          context->cpu->addLdDE(0x0000);

          // add hl, de    ; hl += variable_adress
          context->cpu->addAddHLDE();
        }

      } break;

      default: {
        context->syntaxError("Wrong array parameters count");
        return false;
      }
    }

  } else {
    if (t) {
      context->syntaxError("Undeclared array or unknown function");
      return false;
    } else {
      // ld hl, variable
      context->fixupResolver->addFix(lexeme);
      context->cpu->addLdHL(0x0000);
    }
  }

  return true;
}

void CompilerVariableEmitter::addTempStr(bool atHL) {
  if (atHL) {
    // call GET_NEXT_TEMP_STRING_ADDRESS
    context->cpu->addCall(def_GET_NEXT_TEMP_STRING_ADDRESS);
  } else {
    // ex de, hl
    context->cpu->addExDEHL();
    // call GET_NEXT_TEMP_STRING_ADDRESS
    context->cpu->addCall(def_GET_NEXT_TEMP_STRING_ADDRESS);
    // ex de, hl
    context->codeOptimizer->addByteOptimized(0xEB);
  }
}

bool CompilerVariableEmitter::addAssignment(ActionNode* action) {
  if (action->lexeme->type == Lexeme::type_keyword) {
    if (action->lexeme->value == "TIME") {
      // ld (0xFC9E), hl    ; JIFFY
      context->cpu->addLdiiHL(0xFC9E);

    } else if (action->lexeme->value == "MAXFILES") {
      // ld a, l
      context->cpu->addLdAL();
      // ld ix, MAXFILES
      context->cpu->addLdIX(def_MAXFILES);
      // call CALBAS
      context->cpu->addCall(def_CALBAS);
      // ei
      context->cpu->addEI();

    } else {
      context->syntaxError("Invalid KEYWORD/FUNCTION assignment");
    }

  } else if (action->lexeme->type == Lexeme::type_identifier) {
    if (action->lexeme->isArray ||
        action->lexeme->subtype == Lexeme::subtype_string) {
      // push hl
      context->cpu->addPushHL();

      if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
          action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // push bc
        context->cpu->addPushBC();
      }

      if (!addVarAddress(action)) return false;

      if (action->lexeme->subtype == Lexeme::subtype_string) {
        // pop de
        context->cpu->addPopDE();
        // ex de,hl
        context->cpu->addExDEHL();

        // call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl
        // end of string)
        context->cpu->addCall(def_XBASIC_COPY_STRING);

      } else if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // pop de
        context->cpu->addPopDE();
        // ld (hl),e
        context->cpu->addLdiHLE();
        // inc hl
        context->cpu->addIncHL();
        // ld (hl),d
        context->cpu->addLdiHLD();

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // pop bc
        context->cpu->addPopBC();
        // pop de
        context->cpu->addPopDE();
        // ld (hl),b
        context->cpu->addLdiHLB();
        // inc hl
        context->cpu->addIncHL();
        // ld (hl),e
        context->cpu->addLdiHLE();
        // inc hl
        context->cpu->addIncHL();
        // ld (hl),d
        context->cpu->addLdiHLD();

      } else {
        context->syntaxError("Invalid assignment");
        return false;
      }

    } else {
      // assignment optimization

      if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // ld (var), hl
        context->fixupResolver->addFix(action->lexeme);
        context->cpu->addLdiiHL(0x0000);

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // ld a, b
        context->cpu->addLdAB();
        // ld (var), a
        context->fixupResolver->addFix(action->lexeme);
        context->cpu->addLdiiA(0x0000);
        // ld (var+1), hl
        context->fixupResolver->addFix(action->lexeme)->step = 1;
        context->cpu->addLdiiHL(0x0000);

      } else {
        context->syntaxError("Invalid assignment");
        return false;
      }
    }

  } else {
    context->syntaxError("Invalid constant/expression assignment");
    return false;
  }

  return true;
}