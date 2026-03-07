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
  auto& cpu = *context->cpu;
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
            cpu.addLdHL(0x0000);

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
            cpu.addAddHLHL();
          } else if (lexeme->x_factor == 3) {
            // ld d, h
            cpu.addLdDH();
            // ld e, l
            cpu.addLdEL();
            // add hl, hl
            cpu.addAddHLHL();
            // add hl, de
            cpu.addAddHLDE();
          } else if (lexeme->x_factor == 256) {
            // ld h, l
            cpu.addLdHL();
            // ld l, 0
            cpu.addLdL(0x00);
          } else {
            // ld de, x_factor
            cpu.addLdDE(lexeme->x_factor);

            // call 0x761b    ; integer multiplication (hl = hl * de)
            cpu.addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

          // ld de, variable
          context->fixupResolver->addFix(lexeme);
          cpu.addLdDE(0x0000);

          // add hl, de   ; hl = variable_address +  (x_index * x_factor)
          cpu.addAddHLDE();
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
              cpu.addLdHL(0x0000);

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
            cpu.addAddHLHL();
          } else if (lexeme->x_factor == 3) {
            // ld d, h
            cpu.addLdDH();
            // ld e, l
            cpu.addLdEL();
            // add hl, hl
            cpu.addAddHLHL();
            // add hl, de
            cpu.addAddHLDE();
          } else if (lexeme->x_factor == 256) {
            // ld h, l
            cpu.addLdHL();
            // ld l, 0
            cpu.addLdL(0x00);
          } else {
            // ld de, x_factor
            cpu.addLdDE(lexeme->x_factor);

            // call 0x761b    ; integer multiplication (hl = hl * de)
            cpu.addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

          // push hl
          cpu.addPushHL();

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
            cpu.addLdBH();
            // ld c, l
            cpu.addLdCL();
          }

          while (factor) {
            if (!first) {
              // ex de, hl     ; save current total
              cpu.addExDEHL();
              // ld h, b       ; restore index from bc
              cpu.addLdHB();
              // ld l, c
              cpu.addLdLC();
            }

            diff = 1;

            while (factor >= (diff << 1)) {
              // add hl, hl    ; x 2
              cpu.addAddHLHL();

              diff <<= 1;  // diff *= 2;
            }

            if (first) {
              first = false;
            } else {
              // add hl, de    ; add last total
              cpu.addAddHLDE();
            }

            factor -= diff;
          }

          if (lexeme->x_factor == 2) {
            // add hl, hl    ; x 2
            cpu.addAddHLHL();
          }

          // pop de
          cpu.addPopDE();

          // add hl, de     ; hl = (x_index * x_factor) + (y_index * y_factor)
          cpu.addAddHLDE();

          // ld de, variable
          context->fixupResolver->addFix(lexeme);
          cpu.addLdDE(0x0000);

          // add hl, de    ; hl += variable_adress
          cpu.addAddHLDE();
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
      cpu.addLdHL(0x0000);
    }
  }

  return true;
}

void CompilerVariableEmitter::addTempStr(bool atHL) {
  auto& cpu = *context->cpu;
  if (atHL) {
    // call GET_NEXT_TEMP_STRING_ADDRESS
    cpu.addCall(def_GET_NEXT_TEMP_STRING_ADDRESS);
  } else {
    // ex de, hl
    cpu.addExDEHL();
    // call GET_NEXT_TEMP_STRING_ADDRESS
    cpu.addCall(def_GET_NEXT_TEMP_STRING_ADDRESS);
    // ex de, hl
    context->codeOptimizer->addByteOptimized(0xEB);
  }
}

bool CompilerVariableEmitter::addAssignment(ActionNode* action) {
  auto& cpu = *context->cpu;
  if (action->lexeme->type == Lexeme::type_keyword) {
    if (action->lexeme->value == "TIME") {
      // ld (0xFC9E), hl    ; JIFFY
      cpu.addLdiiHL(0xFC9E);

    } else if (action->lexeme->value == "MAXFILES") {
      // ld a, l
      cpu.addLdAL();
      // ld ix, MAXFILES
      cpu.addLdIX(def_MAXFILES);
      // call CALBAS
      cpu.addCall(def_CALBAS);
      // ei
      cpu.addEI();

    } else {
      context->syntaxError("Invalid KEYWORD/FUNCTION assignment");
    }

  } else if (action->lexeme->type == Lexeme::type_identifier) {
    if (action->lexeme->isArray ||
        action->lexeme->subtype == Lexeme::subtype_string) {
      // push hl
      cpu.addPushHL();

      if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
          action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // push bc
        cpu.addPushBC();
      }

      if (!addVarAddress(action)) return false;

      if (action->lexeme->subtype == Lexeme::subtype_string) {
        // pop de
        cpu.addPopDE();
        // ex de,hl
        cpu.addExDEHL();

        // call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl
        // end of string)
        cpu.addCall(def_XBASIC_COPY_STRING);

      } else if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // pop de
        cpu.addPopDE();
        // ld (hl),e
        cpu.addLdiHLE();
        // inc hl
        cpu.addIncHL();
        // ld (hl),d
        cpu.addLdiHLD();

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // pop bc
        cpu.addPopBC();
        // pop de
        cpu.addPopDE();
        // ld (hl),b
        cpu.addLdiHLB();
        // inc hl
        cpu.addIncHL();
        // ld (hl),e
        cpu.addLdiHLE();
        // inc hl
        cpu.addIncHL();
        // ld (hl),d
        cpu.addLdiHLD();

      } else {
        context->syntaxError("Invalid assignment");
        return false;
      }

    } else {
      // assignment optimization

      if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // ld (var), hl
        context->fixupResolver->addFix(action->lexeme);
        cpu.addLdiiHL(0x0000);

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // ld a, b
        cpu.addLdAB();
        // ld (var), a
        context->fixupResolver->addFix(action->lexeme);
        cpu.addLdiiA(0x0000);
        // ld (var+1), hl
        context->fixupResolver->addFix(action->lexeme)->step = 1;
        cpu.addLdiiHL(0x0000);

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