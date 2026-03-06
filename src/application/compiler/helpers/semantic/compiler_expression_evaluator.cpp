/***
 * @file compiler_expression_evaluator.cpp
 * @brief Compiler expression evaluator
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_expression_evaluator.h"

#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_float_converter.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"

int CompilerExpressionEvaluator::evalExpression(ActionNode* action) {
  int result = Lexeme::subtype_unknown;
  Lexeme* lexeme;

  lexeme = action->lexeme;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_identifier) {
      result = lexeme->subtype;

      if (lexeme->isArray || result == Lexeme::subtype_string) {
        if (!context->variableEmitter->addVarAddress(action)) {
          if (!lexeme->isArray && action->actions.size()) {
            context->syntaxError("Undeclared array or unknown function");
          } else
            result = Lexeme::subtype_unknown;
        } else {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            // ld e, (hl)
            context->cpu->addLdEiHL();
            // inc hl
            context->cpu->addIncHL();
            // ld d, (hl)
            context->cpu->addLdDiHL();
            // ex de, hl
            context->cpu->addExDEHL();
          } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                     lexeme->subtype == Lexeme::subtype_double_decimal) {
            // ld b, (hl)
            context->cpu->addLdBiHL();
            // inc hl
            context->cpu->addIncHL();
            // ld e, (hl)
            context->cpu->addLdEiHL();
            // inc hl
            context->cpu->addIncHL();
            // ld d, (hl)
            context->cpu->addLdDiHL();
            // ex de, hl
            context->cpu->addExDEHL();
          }
        }
      } else {
        if (action->actions.size()) {
          context->syntaxError("Undeclared array or unknown function");
          return result;
        }

        // get numeric variable data optimization

        if (lexeme->subtype == Lexeme::subtype_numeric) {
          // ld hl, (variable)
          context->fixupResolver->addFix(lexeme);
          context->cpu->addLdHLii(0x0000);
        } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                   lexeme->subtype == Lexeme::subtype_double_decimal) {
          // ld a, (variable)
          context->fixupResolver->addFix(lexeme);
          context->cpu->addLdAii(0x0000);
          // ld b, a
          context->cpu->addLdBA();
          // ld hl, (variable+1)
          context->fixupResolver->addFix(lexeme)->step = 1;
          context->cpu->addLdHLii(0x0000);
        }
      }

    } else if (lexeme->type == Lexeme::type_keyword) {
      result = evalFunction(action);

    } else if (lexeme->type == Lexeme::type_operator) {
      result = evalOperator(action);

    } else if (lexeme->type == Lexeme::type_separator) {
      context->syntaxError("Invalid separator on expression");
      result = Lexeme::subtype_numeric;

    } else if (lexeme->type == Lexeme::type_literal) {
      result = lexeme->subtype;

      if (lexeme->subtype == Lexeme::subtype_string) {
        context->fixupResolver->addFix(lexeme);

        context->cpu->addLdHL(0x0000);  // ld hl, string

      } else if (lexeme->subtype == Lexeme::subtype_numeric) {
        char* s = (char*)lexeme->value.c_str();
        try {
          if (s[0] == '&') {
            if (s[1] == 'h' || s[1] == 'H')
              context->cpu->addLdHL(
                  stoi(lexeme->value.substr(2), 0, 16));  // ld hl, value
            else if (s[1] == 'o' || s[1] == 'O')
              context->cpu->addLdHL(
                  stoi(lexeme->value.substr(2), 0, 8));  // ld hl, value
            else if (s[1] == 'b' || s[1] == 'B')
              context->cpu->addLdHL(
                  stoi(lexeme->value.substr(2), 0, 2));  // ld hl, value
            else
              result = Lexeme::subtype_unknown;
          } else
            context->cpu->addLdHL(stoi(lexeme->value));  // ld hl, value
        } catch (exception& e) {
          printf("Warning: error while converting numeric constant %s\n",
                 lexeme->value.c_str());
          context->cpu->addLdHL(0x0000);  // ld hl, value
        }

      } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                 lexeme->subtype == Lexeme::subtype_double_decimal) {
        int value = context->floatConverter->str2FloatLib(lexeme->value);

        context->cpu->addLdB((value >> 16) & 0xff);  // ld b, value
        context->cpu->addLdHL(value & 0xffff);       // ld hl, value

      } else if (lexeme->subtype == Lexeme::subtype_null) {
      } else {
        result = Lexeme::subtype_unknown;
      }
    } else
      result = Lexeme::subtype_unknown;
  }

  return result;
}

int CompilerExpressionEvaluator::evalOperator(ActionNode* action) {
  int result = Lexeme::subtype_unknown;
  Lexeme* lexeme;
  ActionNode* next_action;
  unsigned int t = action->actions.size(), i, n, k;
  unsigned char* s;

  if (!t) return result;

  lexeme = action->lexeme;

  if (lexeme) {
    if (t == 1) {
      if (!evalOperatorParms(action, 1)) return result;

      next_action = action->actions[0];
      result = next_action->subtype;

      if (lexeme->value == "NOT") {
        if (result == Lexeme::subtype_numeric) {
          // call intCompareNOT
          // ctx.cpu->addCall(def_intCompareNOT);
          context->codeOptimizer->addKernelCall(def_intCompareNOT);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "-") {
        if (result == Lexeme::subtype_numeric) {
          // call intNEG
          // ctx.cpu->addCall(def_intNEG);
          context->codeOptimizer->addKernelCall(def_intNEG);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // call floatNeg
          // ctx.cpu->addCall(def_floatNEG);
          context->codeOptimizer->addKernelCall(def_floatNEG);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "+") {
        return result;
      } else
        result = Lexeme::subtype_unknown;

    } else if (t == 2) {
      if (!evalOperatorParms(action, 2)) return result;

      result = evalOperatorCast(action);
      if (result == Lexeme::subtype_unknown) return result;

      if (lexeme->value == "AND") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareAND
          // ctx.cpu->addCall(def_intCompareAND);
          context->codeOptimizer->addKernelCall(def_intCompareAND);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "OR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareOR
          // ctx.cpu->addCall(def_intCompareOR);
          context->codeOptimizer->addKernelCall(def_intCompareOR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "XOR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareXOR
          // ctx.cpu->addCall(def_intCompareXOR);
          context->codeOptimizer->addKernelCall(def_intCompareXOR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "EQV") {
        // same as: not a xor b

        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // ld a, l
          context->cpu->addLdAL();
          // xor e
          context->cpu->addXorE();
          // cpl
          context->cpu->addCPL();
          // ld l, a
          context->cpu->addLdLA();
          // ld a, h
          context->cpu->addAddH();
          // xor d
          context->cpu->addXorD();
          // cpl
          context->cpu->addCPL();
          // ld h, a
          context->cpu->addLdHA();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "IMP") {
        if (result == Lexeme::subtype_numeric) {
          // same as: not a or b

          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // ld a, e
          context->cpu->addLdAE();
          // cpl
          context->cpu->addCPL();
          // or l
          context->cpu->addOrL();
          // ld l, a
          context->cpu->addLdLA();
          // ld a, d
          context->cpu->addLdAD();
          // cpl
          context->cpu->addCPL();
          // or h
          context->cpu->addOrH();
          // ld h, a
          context->cpu->addLdHA();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareEQ
          // ctx.cpu->addCall(def_intCompareEQ);
          context->codeOptimizer->addKernelCall(def_intCompareEQ);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();

          // 7876 xbasic compare floats (=)
          context->cpu->addCall(def_XBASIC_COMPARE_FLOATS_EQ);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          context->cpu->addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          context->cpu->addPopHL();

          // 7eae xbasic compare string (NULBUF = string)
          context->cpu->addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<>") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareNE
          // ctx.cpu->addCall(def_intCompareNE);
          context->codeOptimizer->addKernelCall(def_intCompareNE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();

          // 787f xbasic compare floats (<>)
          context->cpu->addCall(def_XBASIC_COMPARE_FLOATS_NE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          context->cpu->addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          context->cpu->addPopHL();

          // 7ec9 xbasic compare string (NULBUF <> string)
          context->cpu->addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_NE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareLT
          // ctx.cpu->addCall(def_intCompareLT);
          context->codeOptimizer->addKernelCall(def_intCompareLT);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();

          // 7888 xbasic compare floats (>)
          context->cpu->addCall(def_XBASIC_COMPARE_FLOATS_GT);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          context->cpu->addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          context->cpu->addPopHL();

          // 7ea4 xbasic compare string (NULBUF > string)
          context->cpu->addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_GT);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareLE
          // ctx.cpu->addCall(def_intCompareLE);
          context->codeOptimizer->addKernelCall(def_intCompareLE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();

          // 7892 xbasic compare floats (>=)
          context->cpu->addCall(def_XBASIC_COMPARE_FLOATS_GE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          context->cpu->addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          context->cpu->addPopHL();

          // 7eb7 xbasic compare string (NULBUF >= string)
          context->cpu->addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_GE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == ">") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareGT
          // ctx.cpu->addCall(def_intCompareGT);
          context->codeOptimizer->addKernelCall(def_intCompareGT);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();

          // 789b xbasic compare floats (<)
          context->cpu->addCall(def_XBASIC_COMPARE_FLOATS_LT);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          context->cpu->addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          context->cpu->addPopHL();

          // 7ec0 xbasic compare string (NULBUF < string)
          context->cpu->addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_LT);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == ">=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intCompareGE
          // ctx.cpu->addCall(def_intCompareGE);
          context->codeOptimizer->addKernelCall(def_intCompareGE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();

          // 78a4 xbasic compare floats (<=)
          context->cpu->addCall(def_XBASIC_COMPARE_FLOATS_LE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          context->cpu->addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          context->cpu->addPopHL();

          // 7ed2 xbasic compare string (NULBUF <= string)
          context->cpu->addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_LE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "+") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);
          // add hl, de      ; add integers (math optimized)
          context->codeOptimizer->addByteOptimized(0x19);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();
          // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
          context->cpu->addCall(def_XBASIC_ADD_FLOATS);

        } else if (result == Lexeme::subtype_string) {
          // pop bc                      ; bc=string 1, hl=string 2
          context->cpu->addPopBC();
          // ld de, temporary variable   ; de=string destination
          context->variableEmitter->addTempStr(false);
          // push de
          context->cpu->addPushDE();
          //   call 0x7f05               ; xbasic concat strings (in: bc=str1,
          //   hl=str2, de=strdest; out: hl fake)
          context->cpu->addCall(def_XBASIC_CONCAT_STRINGS);
          // pop hl                      ; correct destination
          context->cpu->addPopHL();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "-") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);
          // ex de,hl
          context->codeOptimizer->addByteOptimized(0xEB);

          s = context->cpu->context->code_pipeline[0];
          i = s[1] | (s[2] << 8);
          if (s[0] == 0x11 && i <= 4) {  // ld de, n
            context->cpu->context->code_pointer -= 3;
            context->cpu->context->code_size -= 3;
            while (i) {
              // dec hl
              context->cpu->addDecHL();
              i--;
            }
          } else {
            // and a
            context->cpu->addAndA();
            // sbc hl, de      ; subtract integers
            context->cpu->addSbcHLDE();
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ex de,hl
          context->cpu->addExDEHL();
          // ld c, b
          context->cpu->addLdCB();
          // ld b, a
          context->cpu->addLdBA();
          // call 0x76bd     ; subtract floats (b:hl - c:de = b:hl)
          context->cpu->addCall(def_XBASIC_SUBTRACT_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "*") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          /// @remark math optimization when second parameter is a integer
          /// constant
          if (context->opts->megaROM)
            s = context->cpu->context->code_pipeline[1];
          else
            s = context->cpu->context->code_pipeline[0];

          i = s[1] | (s[2] << 8);

          if (action->actions[0]->lexeme->type == Lexeme::type_literal &&
              i <= 256) {
            if (context->opts->megaROM) {
              context->cpu->context->code_pointer -=
                  5;  //! @todo verify if 5 or 6
              context->cpu->context->code_size -= 5;
            } else {
              context->cpu->context->code_pointer -= 4;
              context->cpu->context->code_size -= 4;

              if (action->actions[1]->lexeme->type == Lexeme::type_literal) {
                context->cpu->context->code_pointer += 1;
                context->cpu->context->code_size += 1;
                s = &context->cpu->context
                         ->code[context->cpu->context->code_pointer - 3];
                if (s[0] == 0x11) s[0] = 0x21;  // change "ld de,n" to "ld hl,n"
              }
            }

            switch (i) {
              case 0: {
                context->cpu->addLdHL(0x0000);
              } break;

              case 128: {
                // XOR A | SRL H | RR L | RRA | LD H, L | LD L, A
                context->cpu->addXorA();
                context->cpu->addSRLH();
                context->cpu->addRRL();
                context->cpu->addRRA();
                context->cpu->addLdHL();
                context->cpu->addLdLA();
              } break;

              case 256: {
                //  LD H, L | LD L, 0
                context->cpu->addLdHL();
                context->cpu->addLdL(0);
              } break;

              default: {
                char b[100];
                n = 0;
                k = 0;
                while (i > 1 && n < 100) {
                  b[n] = (i & 1);
                  if (b[n]) k++;
                  i >>= 1;
                  n++;
                }
                if (k) {
                  context->cpu->addLdEL();
                  context->cpu->addLdDH();
                }
                while (n) {
                  n--;
                  context->cpu->addAddHLHL();
                  if (b[n]) {
                    context->cpu->addAddHLDE();
                  }
                }
              } break;
            }

          } else {
            // call 0x761b     ; multiply integers (hl = hl * de)
            context->cpu->addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ld c, a
          context->cpu->addLdCA();
          // call 0x7732     ; multiply floats
          context->cpu->addCall(def_XBASIC_MULTIPLY_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "/") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);
          // ex de,hl
          context->codeOptimizer->addByteOptimized(0xEB);

          // math optimization when second parameter is a integer constant
          s = context->cpu->context->code_pipeline[0];
          if (s[0] == 0x11) {  // ld de, n
            i = s[1] | (s[2] << 8);

            switch (i) {
              case 0: {
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                context->cpu->addLdHL(0x0000);
              } break;

              case 1: {
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
              } break;

              case 2: {
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // sra h | rr l
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // JRNC $+6   ; jump if there's no rest of division by 2
                context->cpu->addJrNC(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 4: {
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD A, L
                context->cpu->addLdAL();
                // sra h | rr l (2 times)
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // and 0x03
                context->cpu->addAnd(0x03);
                // JRZ $+6   ; jump if there's no rest of division by 4
                context->cpu->addJrZ(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 8: {
                // OLD: LD A, L | SRA H | RRA | SRL H | RRA | SRL H | RRA | LD
                // L, A
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD A, L
                context->cpu->addLdAL();
                // sra h | rr l (3 times)
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // and 0x07
                context->cpu->addAnd(0x07);
                // JRZ $+6   ; jump if there's no rest of division by 8
                context->cpu->addJrZ(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 16: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL
                // | RLA | ADD HL, HL | RLA | LD L, H | LD H, A
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD A, L
                context->cpu->addLdAL();
                // sra h | rr l (4 times)
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // and 0x0F
                context->cpu->addAnd(0x0F);
                // JRZ $+6   ; jump if there's no rest of division by 16
                context->cpu->addJrZ(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 32: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL
                // | RLA | LD L, H | LD H, A
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD A, L
                context->cpu->addLdAL();
                // sra h | rr l (5 times)
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // and 0x1F
                context->cpu->addAnd(0x1F);
                // JRZ $+6   ; jump if there's no rest of division by 32
                context->cpu->addJrZ(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 64: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | LD L, H |
                // LD H, A
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD A, L
                context->cpu->addLdAL();
                // sra h | rr l (6 times)
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // and 0x3F
                context->cpu->addAnd(0x3F);
                // JRZ $+6   ; jump if there's no rest of division by 64
                context->cpu->addJrZ(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 128: {
                // old: XOR A | ADD HL, HL | RLA | LD L, H | LD H, A
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD A, L
                context->cpu->addLdAL();
                // sra h | rr l (7 times)
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                context->cpu->addSRAH();
                context->cpu->addRRL();
                // and 0x7F
                context->cpu->addAnd(0x7F);
                // JRZ $+6   ; jump if there's no rest of division by 64
                context->cpu->addJrZ(0x06);
                //   LD A, H
                context->cpu->addLdAH();
                //   AND 0x80  ; sign bit
                context->cpu->addAnd(0x80);
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              case 256: {
                context->cpu->context->code_pointer -= 3;
                context->cpu->context->code_size -= 3;
                // LD E, L
                context->cpu->addLdEL();
                // LD L, H
                context->cpu->addLdLH();
                // LD H, 0x00
                context->cpu->addLdH(0x00);
                // LD A, L
                context->cpu->addLdAL();
                // AND 0x80    ; sign bit
                context->cpu->addAnd(0x80);
                // JRZ $+7
                context->cpu->addJrZ(0x07);
                //   LD H, 0xFF
                context->cpu->addLdH(0xFF);
                //   XOR A
                context->cpu->addXorA();
                //   OR E
                context->cpu->addOrE();
                //   JRZ $+1
                context->cpu->addJrZ(0x01);
                //     INC HL
                context->cpu->addIncHL();
              } break;

              default: {
                // call divide integers
                context->cpu->addCall(def_XBASIC_DIVIDE_INTEGERS);
              } break;
            }

          } else {
            // call divide integers
            context->cpu->addCall(def_XBASIC_DIVIDE_INTEGERS);
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ex de,hl
          context->cpu->addExDEHL();
          // ld c, b
          context->cpu->addLdCB();
          // ld b, a
          context->cpu->addLdBA();
          // call 0x7775     ; divide floats
          context->cpu->addCall(def_XBASIC_DIVIDE_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "\\") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);
          // ex de,hl
          context->codeOptimizer->addByteOptimized(0xEB);
          // call 0x762d     ; divide integers
          context->cpu->addCall(def_XBASIC_DIVIDE_INTEGERS);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop bc
          context->cpu->addPopBC();
          // pop de
          context->cpu->addPopDE();
          // push hl
          context->cpu->addPushHL();
          // ex de, hl
          context->cpu->addExDEHL();

          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop de
          context->cpu->addPopDE();

          // call 0x762d     ; divide integers
          context->cpu->addCall(def_XBASIC_DIVIDE_INTEGERS);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "^") {
        if (result == Lexeme::subtype_numeric) {
          // ex de,hl
          context->codeOptimizer->addByteOptimized(0xEB);

          // pop hl
          context->cpu->addPopHL();

          //   call 0x782D     ; integer to float
          context->cpu->addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT);

          // call 0x77C1       ; power float ^ integer
          context->cpu->addCall(def_XBASIC_POWER_FLOAT_TO_INTEGER);

          result = Lexeme::subtype_single_decimal;

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          context->cpu->addPopAF();
          // pop de
          context->cpu->addPopDE();
          // ex de,hl
          context->cpu->addExDEHL();
          // ld c, b
          context->cpu->addLdCB();
          // ld b, a
          context->cpu->addLdBA();
          // call 0x780d      ; power float ^ float
          context->cpu->addCall(def_XBASIC_POWER_FLOAT_TO_FLOAT);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "MOD") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);
          // ex de,hl
          context->codeOptimizer->addByteOptimized(0xEB);
          // call 0x762d     ; divide integers
          context->cpu->addCall(def_XBASIC_DIVIDE_INTEGERS);
          // ex de, hl       ; remainder
          context->cpu->addExDEHL();

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop bc
          context->cpu->addPopBC();
          // pop de
          context->cpu->addPopDE();
          // push hl
          context->cpu->addPushHL();
          // ex de, hl
          context->cpu->addExDEHL();

          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop de
          context->cpu->addPopDE();

          // call 0x762d     ; divide integers
          context->cpu->addCall(def_XBASIC_DIVIDE_INTEGERS);

          // ex de, hl       ; remainder
          context->cpu->addExDEHL();

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "SHR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intSHR
          // ctx.cpu->addCall(def_intSHR);
          context->codeOptimizer->addKernelCall(def_intSHR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "SHL") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          context->codeOptimizer->addByteOptimized(0xD1);

          // call intSHL
          // ctx.cpu->addCall(def_intSHL);
          context->codeOptimizer->addKernelCall(def_intSHL);

        } else
          result = Lexeme::subtype_unknown;

      } else
        result = Lexeme::subtype_unknown;

    } else
      result = Lexeme::subtype_unknown;
  }

  return result;
}

bool CompilerExpressionEvaluator::evalOperatorParms(ActionNode* action,
                                                    int parmCount) {
  bool result = false;
  int subtype;
  ActionNode* next_action;
  int i, t = action->actions.size();

  if (t == parmCount) {
    result = true;
    for (i = t - 1; i >= 0 && result; i--) {
      next_action = action->actions[i];

      subtype = evalExpression(next_action);
      result &= (subtype != Lexeme::subtype_unknown);

      next_action->subtype = subtype;

      if (i) {
        // push hl
        context->cpu->addPushHL();
        if (subtype == Lexeme::subtype_single_decimal ||
            subtype == Lexeme::subtype_double_decimal) {
          // push bc
          context->cpu->addPushBC();
        }
      }
    }
  }

  return result;
}

int CompilerExpressionEvaluator::evalOperatorCast(ActionNode* action) {
  int result = Lexeme::subtype_unknown;
  ActionNode *next_action1, *next_action2;

  next_action1 = action->actions[0];
  next_action2 = action->actions[1];

  if (next_action1->subtype == next_action2->subtype) {
    result = next_action1->subtype;

  } else if (next_action2->subtype == Lexeme::subtype_numeric &&
             (next_action1->subtype == Lexeme::subtype_single_decimal ||
              next_action1->subtype == Lexeme::subtype_double_decimal)) {
    // ex: (float) + (int)
    //     200.0 + 100

    // call castParamFloatInt
    context->cpu->addCall(def_castParamFloatInt);

    result = next_action1->subtype;

  } else if (next_action1->subtype == Lexeme::subtype_numeric &&
             (next_action2->subtype == Lexeme::subtype_single_decimal ||
              next_action2->subtype == Lexeme::subtype_double_decimal)) {
    // ex: (int) + (float)
    // ex: 200 + 100.0

    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    result = next_action2->subtype;

  } else if (next_action2->subtype == Lexeme::subtype_numeric &&
             next_action1->subtype == Lexeme::subtype_string) {
    // pop de   ; swap parameters code
    context->cpu->addPopDE();
    // push hl
    context->cpu->addPushHL();
    // ex de,hl
    context->cpu->addExDEHL();

    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    // pop de   ; swap again
    context->cpu->addPopDE();
    // push hl
    context->cpu->addPushHL();
    // ex de,hl
    context->cpu->addExDEHL();

    result = next_action2->subtype;

  } else if (next_action1->subtype == Lexeme::subtype_numeric &&
             next_action2->subtype == Lexeme::subtype_string) {
    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    result = next_action2->subtype;

  } else if ((next_action2->subtype == Lexeme::subtype_single_decimal ||
              next_action2->subtype == Lexeme::subtype_double_decimal) &&
             next_action1->subtype == Lexeme::subtype_string) {
    result = Lexeme::subtype_unknown;

  } else if ((next_action1->subtype == Lexeme::subtype_single_decimal ||
              next_action1->subtype == Lexeme::subtype_double_decimal) &&
             next_action2->subtype == Lexeme::subtype_string) {
    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    result = next_action2->subtype;

  } else if ((next_action1->subtype == Lexeme::subtype_single_decimal &&
              next_action2->subtype == Lexeme::subtype_double_decimal) ||
             (next_action2->subtype == Lexeme::subtype_single_decimal &&
              next_action1->subtype == Lexeme::subtype_double_decimal)) {
    result = Lexeme::subtype_double_decimal;

  } else
    result = Lexeme::subtype_unknown;

  return result;
}

void CompilerExpressionEvaluator::addCast(int from, int to) {
  if (from != to) {
    if (from == Lexeme::subtype_numeric) {
      if (to == Lexeme::subtype_numeric) {
        return;
      } else if (to == Lexeme::subtype_string) {
        // call 0x7b26   ; xbasic int to string (in hl, out hl)
        context->cpu->addCall(def_XBASIC_CAST_INTEGER_TO_STRING);
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        // call 0x782d   ; xbasic int to float (in hl, out b:hl)
        context->cpu->addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT);
      }

    } else if (from == Lexeme::subtype_string) {
      if (to == Lexeme::subtype_numeric) {
        // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
        // b:hl)
        context->cpu->addCall(def_XBASIC_CAST_STRING_TO_FLOAT);
        // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
        context->cpu->addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER);
      } else if (to == Lexeme::subtype_string) {
        return;
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
        // b:hl)
        context->cpu->addCall(def_XBASIC_CAST_STRING_TO_FLOAT);
      }

    } else if (from == Lexeme::subtype_single_decimal ||
               from == Lexeme::subtype_double_decimal) {
      if (to == Lexeme::subtype_numeric) {
        // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
        context->cpu->addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER);
      } else if (to == Lexeme::subtype_string) {
        // call 0x7b80   ; xbasic float to string (in b:hl, out hl)
        context->cpu->addCall(def_XBASIC_CAST_FLOAT_TO_STRING);
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        return;
      }

    } else {
      context->syntaxError("Unknown type to cast");
    }
  }
}

int CompilerExpressionEvaluator::evalFunction(ActionNode* action) {
  int result[4];
  Lexeme *lexeme, *lexeme2;
  ActionNode* next_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < 4; i++) result[i] = Lexeme::subtype_unknown;

  lexeme = action->lexeme;

  if (lexeme) {
    if (t) {
      if (lexeme->value == "VARPTR") {
        next_action = action->actions[0];
        if (next_action->lexeme->type == Lexeme::type_identifier) {
          context->variableEmitter->addVarAddress(next_action);
          return Lexeme::subtype_numeric;
        } else
          return Lexeme::subtype_unknown;
      } else if (lexeme->value == "USING$") {
        if (t >= 2) {
          next_action = action->actions[1];
          lexeme2 = next_action->lexeme;
          if (lexeme2) {
            if (lexeme2->type == Lexeme::type_literal &&
                lexeme2->subtype == Lexeme::subtype_string) {
              int r = context->floatConverter->getUsingFormat(lexeme2->value);
              lexeme2->subtype = Lexeme::subtype_numeric;
              lexeme2->value = to_string(r);
            }
          }
        }
      }

      if (!evalOperatorParms(action, t)) return result[0];

      for (i = 0; i < t; i++) {
        next_action = action->actions[i];
        result[i] = next_action->subtype;

        if (result[i] == Lexeme::subtype_unknown) return result[i];
      }
    }

    switch (t) {
      case 0: {
        if (lexeme->value == "TIME") {
          // ld hl, (0xFC9E)    ; JIFFY
          context->cpu->addLdHLii(0xFC9E);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "POS") {
          // ld hl, (0xF661)  ; TTYPOS
          context->cpu->addLdHLii(0xF661);
          // ld h, 0
          context->cpu->addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "LPOS") {
          // ld hl, (0xF415)  ; LPTPOS
          context->cpu->addLdHLii(0xF415);
          // ld h, 0
          context->cpu->addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "CSRLIN") {
          // ld hl, (0xF3DC)  ; CSRY
          context->cpu->addLdHLii(def_CSRY);
          // ld h, 0
          context->cpu->addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "INKEY") {
          // ld hl, 0
          context->cpu->addLdHL(0x0000);
          // call 0x009C        ; CHSNS
          context->cpu->addCall(0x009C);
          // jr z,$+5
          context->cpu->addJrZ(0x04);
          //   call 0x009F        ; CHGET
          context->cpu->addCall(0x009F);
          //   ld l, a
          context->cpu->addLdLA();

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "INKEY$") {
          // call 0x009C        ; CHSNS
          context->cpu->addCall(0x009C);
          // ld hl, temporary string
          context->variableEmitter->addTempStr(true);
          // call 0x7e5e   ; xbasic INKEY$ (in: hl=dest; out: hl=result)
          context->cpu->addCall(def_XBASIC_INKEY);

          result[0] = Lexeme::subtype_string;

        } else if (lexeme->value == "MAXFILES") {
          // ld hl, (MAXFIL)
          context->cpu->addLdHLii(0xF85F);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "FRE") {
          // ld hl, (HEAPSIZ)
          context->cpu->addLdHLii(def_HEAPSIZ);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "HEAP") {
          // ld hl, (HEAPSTR)
          context->cpu->addLdHLii(def_HEAPSTR);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "MSX") {
          // ld hl, (VERSION)                    ; 0 = MSX1, 1 = MSX2, 2 =
          // MSX2+, 3 = MSXturboR
          context->cpu->addLdHLii(def_VERSION);
          // ld h, 0
          context->cpu->addLdH(0x00);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "NTSC") {
          // ld hl, 0
          context->cpu->addLdHL(0x0000);
          // ld a, (NTSC)
          context->cpu->addLdAii(def_NTSC);
          // and 128   ; bit 7 on?
          context->cpu->addAnd(0x80);
          // jr nz, $+1
          context->cpu->addJrNZ(0x01);
          //    dec hl
          context->cpu->addDecHL();

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "VDP") {
          // VDP() without parameters returns VDP version
          // ld a, 4
          context->cpu->addLdA(4);
          // CALL USR2
          context->cpu->addCall(
              context->codeOptimizer->getKernelCallAddr(def_usr2) + 1);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "TURBO") {
          // ld a, 5
          context->cpu->addLdA(5);
          // CALL USR2
          context->cpu->addCall(
              context->codeOptimizer->getKernelCallAddr(def_usr2) + 1);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "COLLISION") {
          // CALL SUB_SPRCOL_ALL
          context->cpu->addCall(def_usr3_COLLISION_ALL);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "MAKER") {
          // ld a, 6
          context->cpu->addLdA(6);
          // CALL USR2
          context->cpu->addCall(
              context->codeOptimizer->getKernelCallAddr(def_usr2) + 1);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "PLYSTATUS") {
          // CALL usr2_player_status
          context->cpu->addCall(def_usr2_player_status);

          result[0] = Lexeme::subtype_numeric;

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;

      case 1: {
        if (lexeme->value == "INT") {
          if (result[0] == Lexeme::subtype_numeric) {
            // its ok, return same parameter value
            return result[0];

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // call 0x78e5         ; xbasic INT
            context->cpu->addCall(def_XBASIC_INT);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "FIX") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x78d8         ; xbasic FIX (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_FIX);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "RND") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7678         ; xbasic RND (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_RND);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SIN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7936         ; xbasic SIN (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_SIN);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "COS") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x792e         ; xbasic COS (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_COS);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "TAN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7990         ; xbasic TAN (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_TAN);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "ATN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x79b2         ; xbasic ATN (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_ATN);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "EXP") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x79fa         ; xbasic EXP (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_EXP);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "LOG") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7a53         ; xbasic LOG (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_LOG);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SQR") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7ab5         ; xbasic SQR (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_SQR);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SGN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x5b5d         ; xbasic SGN (in hl, out hl)
            context->cpu->addCall(def_XBASIC_SGN_INT);

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // call 0x5b72         ; xbasic SGN (in b:hl, out b:hl)
            context->cpu->addCall(def_XBASIC_SGN_FLOAT);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "ABS") {
          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x5b36         ; xbasic ABS (in hl, out hl)
            context->cpu->addCall(def_XBASIC_ABS_INT);

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // xbasic ABS (in b:hl, out b:hl)
            // res 7,h
            context->cpu->addWord(0xCB, 0xBC);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "VAL") {
          if (result[0] == Lexeme::subtype_numeric) {
            // its ok, return same parameter value
            return result[0];

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // its ok, return same parameter value
            return result[0];

          } else if (result[0] == Lexeme::subtype_string) {
            // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
            // b:hl)
            context->cpu->addCall(def_XBASIC_VAL);
            result[0] = Lexeme::subtype_single_decimal;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PEEK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld l,(hl)
            context->cpu->addLdLiHL();
            // ld h, 0
            context->cpu->addLdH(0x00);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "IPEEK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld e,(hl)
            context->cpu->addLdEiHL();
            // inc HL
            context->cpu->addIncHL();
            // ld d, (hl)
            context->cpu->addLdDiHL();
            // ex de, hl
            context->cpu->addExDEHL();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "VPEEK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x70a1    ; xbasic VPEEK (in:hl, out:hl)
            context->cpu->addCall(def_XBASIC_VPEEK);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "INP") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, l
            context->cpu->addLdCL();
            // in a, (c)
            context->cpu->addWord(0xED, 0x78);
            // ld l, a
            context->cpu->addLdLA();
            // ld h, 0
            context->cpu->addLdH(0x00);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "EOF") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld (DAC+2), hl
            context->cpu->addLdiiHL(def_DAC + 2);

            context->codeHelper->addEnableBasicSlot();

            // call HSAVD          ; alloc disk
            context->cpu->addCall(0xFE94);
            // call GETIOBLK       ; get io channel control block from DAC
            context->cpu->addCall(0x6A6A);
            // jr z, $+6           ; file not open
            context->cpu->addJrZ(0x05);
            // jr c, $+4           ; not a disk drive device
            context->cpu->addJrC(0x03);
            // call HEOF           ; put in DAC end of file status
            context->cpu->addCall(0xFEA3);

            context->codeHelper->addDisableBasicSlot();

            // ld hl, (DAC+2)
            context->cpu->addLdHLii(def_DAC + 2);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "VDP") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x7337         ; xbasic VDP (in: hl, out: hl)
            context->cpu->addCall(def_XBASIC_VDP);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PSG") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // cp 16
            context->cpu->addCp(0x10);
            // jr nc, $+4
            context->cpu->addJrNC(0x03);
            //   call 0x0096         ; RDPSG (in: a = PSG register)
            context->cpu->addCall(0x0096);
            //   ld l, a
            context->cpu->addLdLA();
            //   ld h, 0
            context->cpu->addLdH(0x00);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PLAY") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call usr2_play
            context->cpu->addCall(def_usr2_play);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STICK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // call 0x00D5      ; GTSTCK
            context->cpu->addCall(0x00D5);
            // ld h, 0
            context->cpu->addLdH(0x00);
            // ld l, a
            context->cpu->addLdLA();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STRIG") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // call 0x00D8      ; GTTRIG
            context->cpu->addCall(0x00D8);
            // ld h, a
            context->cpu->addLdHA();
            // ld l, a
            context->cpu->addLdLA();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PAD") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call cmd_pad
            context->cpu->addCall(def_cmd_pad);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PDL") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // call 0x00DE      ; GTPDL
            context->cpu->addCall(0x00DE);
            // ld h, 0
            context->cpu->addLdH(0x00);
            // ld l, a
            context->cpu->addLdLA();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "BASE") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call base function
            context->cpu->addCall(def_XBASIC_BASE);
            // ctx.cpu->addCall(def_XBASIC_ABS_INT);  // abs()

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "ASC") {
          if (result[0] == Lexeme::subtype_string) {
            // ex de, hl
            context->codeOptimizer->addByteOptimized(0xEB);
            // ld hl, 0
            context->cpu->addLdHL(0x0000);
            // ld a, (de)
            context->cpu->addLdAiDE();
            // and a
            context->cpu->addAndA();
            // jr z,$+4
            context->cpu->addJrZ(0x03);
            //   inc de
            context->cpu->addIncDE();
            //   ld a, (de)
            context->cpu->addLdAiDE();
            //   ld l, a
            context->cpu->addLdLA();

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "LEN") {
          if (result[0] == Lexeme::subtype_string) {
            // ld l, (hl)
            context->cpu->addLdLiHL();
            // ld h, 0
            context->cpu->addLdH(0x00);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "CSNG") {
          // cast
          addCast(result[0], Lexeme::subtype_single_decimal);

          result[0] = Lexeme::subtype_single_decimal;

        } else if (lexeme->value == "CDBL") {
          // cast
          addCast(result[0], Lexeme::subtype_double_decimal);

          result[0] = Lexeme::subtype_double_decimal;

        } else if (lexeme->value == "CINT") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "POS") {
          // ld hl, (0xF661)  ; TTYPOS
          context->cpu->addLdHLii(0xF661);
          // ld h, 0
          context->cpu->addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "LPOS") {
          // ld hl, (0xF415)  ; LPTPOS
          context->cpu->addLdHLii(0xF415);
          // ld h, 0
          context->cpu->addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "CSRLIN") {
          // ld hl, (0xF3DC)  ; CSRY
          context->cpu->addLdHLii(0xF3DC);
          // ld h, 0
          context->cpu->addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "CHR$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ex de, hl
            context->codeOptimizer->addByteOptimized(0xEB);
            // ld hl, temporary string
            context->variableEmitter->addTempStr(true);
            // inc d
            context->cpu->addIncD();
            // ld (hl), d
            context->cpu->addLdiHLD();
            // inc hl
            context->cpu->addIncHL();
            // ld (hl), e
            context->cpu->addLdiHLE();
            // dec hl
            context->cpu->addDecHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SPACE$" || lexeme->value == "SPC") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, 0x20      ; space
            context->cpu->addLdA(0x20);
            // ld b, l
            context->cpu->addLdBL();
            // ld hl, temporary string
            context->variableEmitter->addTempStr(true);
            // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars,
            // a=char)
            context->cpu->addCall(def_XBASIC_STRING);

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "TAB") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call tab function
            context->cpu->addCall(def_XBASIC_TAB);

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STR$") {
          // cast
          addCast(result[0], Lexeme::subtype_string);

          result[0] = Lexeme::subtype_string;

        } else if (lexeme->value == "INPUT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ex de, hl
            context->codeOptimizer->addByteOptimized(0xEB);
            // ld hl, temporary string
            context->variableEmitter->addTempStr(true);
            // push hl
            context->cpu->addPushHL();
            //   ld (hl), e
            context->cpu->addLdiHLE();
            //   inc hl
            context->cpu->addIncHL();
            //     call 0x009F        ; CHGET
            context->cpu->addCall(0x009F);
            //     ld (hl), a
            context->cpu->addLdiHLA();
            //     inc hl
            context->cpu->addIncHL();
            //     dec e
            context->cpu->addDecE();
            //   jr nz,$-8
            context->cpu->addJrNZ((unsigned char)(0xFF - 7));
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "BIN$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, 1
            context->cpu->addLdC(0x01);
            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
            // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
            context->cpu->addCall(def_XBASIC_OCT_HEX_BIN);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "OCT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, 3
            context->cpu->addLdC(0x03);
            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
            // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
            context->cpu->addCall(def_XBASIC_OCT_HEX_BIN);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "HEX$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, 4
            context->cpu->addLdC(0x04);
            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
            // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
            context->cpu->addCall(def_XBASIC_OCT_HEX_BIN);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "RESOURCE") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          // call usr0
          context->cpu->addCall(def_usr0);

        } else if (lexeme->value == "RESOURCESIZE") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          // call usr1
          context->cpu->addCall(def_usr1);

        } else if (lexeme->value == "COLLISION") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          // call SUB_SPRCOL_ONE
          context->cpu->addCall(def_usr3_COLLISION_ONE);

        } else if (lexeme->value == "USR" || lexeme->value == "USR0") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // xor a
            context->cpu->addXorA();
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          } else {
            // call usr0
            context->cpu->addCall(def_usr0);
          }

        } else if (lexeme->value == "USR1") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 1
            context->cpu->addLdA(0x01);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          } else {
            // call usr1
            context->cpu->addCall(def_usr1);
          }

        } else if (lexeme->value == "USR2") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 2
            context->cpu->addLdA(0x02);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          } else {
            // call usr2
            context->cpu->addCall(def_usr2);
          }

        } else if (lexeme->value == "USR3") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 3
            context->cpu->addLdA(0x03);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          } else {
            // call usr3
            context->cpu->addCall(def_usr3);
          }

        } else if (lexeme->value == "USR4") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 4
            context->cpu->addLdA(0x04);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR5") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 5
            context->cpu->addLdA(0x05);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR6") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 6
            context->cpu->addLdA(0x06);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR7") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 7
            context->cpu->addLdA(0x07);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR8") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 8
            context->cpu->addLdA(0x08);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR9") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (context->has_defusr) {
            // ld a, 9
            context->cpu->addLdA(0x09);
            // call XBASIC_USR
            context->cpu->addCall(def_XBASIC_USR);
          }

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;

      case 2: {
        if (lexeme->value == "POINT") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // ex de,hl
            context->codeOptimizer->addByteOptimized(0xEB);
            // pop bc
            context->cpu->addPopBC();
            // pop hl
            context->cpu->addPopHL();
            // push de
            context->cpu->addPushDE();

            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            result[1] = Lexeme::subtype_numeric;

            // pop de
            context->cpu->addPopDE();
            // ex de,hl
            context->cpu->addExDEHL();

          } else {
            // pop de
            context->cpu->addPopDE();
          }

          if (result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // call 0x6fa7     ; xbasic POINT (in: de=x, hl=y; out: hl=color)
            context->cpu->addCall(def_XBASIC_POINT);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "TILE") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // ex de,hl
            context->codeOptimizer->addByteOptimized(0xEB);
            // pop bc
            context->cpu->addPopBC();
            // pop hl
            context->cpu->addPopHL();
            // push de
            context->cpu->addPushDE();

            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            result[1] = Lexeme::subtype_numeric;

            // pop de
            context->cpu->addPopDE();
            // ex de,hl
            context->cpu->addExDEHL();

          } else {
            // pop de
            context->cpu->addPopDE();
          }

          if (result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // ld h, e
            context->cpu->addLdHE();
            // inc l
            context->cpu->addIncL();
            // inc h     ; bios based coord system (home=1,1)
            context->cpu->addIncH();

            // call def_tileAddress (in: hl=xy; out: hl=address)
            context->cpu->addCall(def_tileAddress);

            // call 0x70a1    ; xbasic VPEEK (in:hl, out:hl)
            context->cpu->addCall(def_XBASIC_VPEEK);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "COLLISION") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // ex de,hl
            context->codeOptimizer->addByteOptimized(0xEB);
            // pop bc
            context->cpu->addPopBC();
            // pop hl
            context->cpu->addPopHL();
            // push de
            context->cpu->addPushDE();

            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            result[1] = Lexeme::subtype_numeric;

            // pop de
            context->cpu->addPopDE();
            // ex de,hl
            context->cpu->addExDEHL();

          } else {
            // pop de
            context->cpu->addPopDE();
          }

          if (result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // call SUB_SPRCOL_COUPLE
            context->cpu->addCall(def_usr3_COLLISION_COUPLE);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STRING$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          } else if (result[0] == Lexeme::subtype_string) {
            // inc hl
            context->cpu->addIncHL();
            // ld l, (hl)
            context->cpu->addLdLiHL();
            // ld h, 0
            context->cpu->addLdH(0x00);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_numeric) {
            // pop de
            context->cpu->addPopDE();
          } else if (result[1] == Lexeme::subtype_single_decimal ||
                     result[1] == Lexeme::subtype_double_decimal) {
            // pop bc
            context->cpu->addPopAF();
            // pop de
            context->cpu->addPopDE();
            // push hl
            context->cpu->addPushHL();
            //   ex de,hl
            context->cpu->addExDEHL();
            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            //   ex de,hl
            context->cpu->addExDEHL();
            // pop hl
            context->cpu->addPopHL();
            result[1] = Lexeme::subtype_numeric;
          } else
            result[1] = Lexeme::subtype_unknown;

          if (result[0] == Lexeme::subtype_numeric &&
              result[1] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // ld b, e
            context->cpu->addLdBE();
            // ld hl, temporary string
            context->variableEmitter->addTempStr(true);
            // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars,
            // a=char)
            context->cpu->addCall(def_XBASIC_STRING);

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "LEFT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // pop hl
            context->cpu->addPopHL();

            // call 0x7d99     ; xbasic left string (in: a=size, hl=source; out:
            // hl=BUF)
            context->cpu->addCall(def_XBASIC_LEFT);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "RIGHT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            context->cpu->addLdAL();
            // pop hl
            context->cpu->addPopHL();

            // call 0x7da0     ; xbasic right string (in: a=size, hl=source;
            // out: hl=BUF)
            context->cpu->addCall(def_XBASIC_RIGHT);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "MID$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_numeric) {
            // ld b, l         ; start char
            context->cpu->addLdBL();
            // pop hl          ; source string
            context->cpu->addPopHL();

            // ld a, 0xff      ; number of chars (all left on source string)
            context->cpu->addLdA(0xff);

            // call 0x7db1     ; xbasic mid string (in: b=start, a=size,
            // hl=source; out: hl=BUF)
            context->cpu->addCall(def_XBASIC_MID);

            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "USING$") {
          if (result[0] == Lexeme::subtype_double_decimal ||
              result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_single_decimal) {
            // ld c, b
            context->cpu->addLdCB();
            // ex de, hl
            context->cpu->addExDEHL();
            // pop hl
            context->cpu->addPopHL();

            // call XBASIC_USING    ; hl = item format string, c:de = float, out
            // hl=string
            context->cpu->addCall(def_XBASIC_USING);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else if (result[1] == Lexeme::subtype_numeric &&
                     result[0] == Lexeme::subtype_single_decimal) {
            // pop de
            context->cpu->addPopDE();
            // push de
            context->cpu->addPushDE();
            //   ld a, e
            context->cpu->addLdAE();
            //   rrca
            context->cpu->addRRCA();
            //   rrca
            context->cpu->addRRCA();
            //   rrca
            context->cpu->addRRCA();
            //   rrca
            context->cpu->addRRCA();
            //   and 0x0F
            context->cpu->addAnd(0x0F);
            //   ld d, a
            context->cpu->addLdDA();
            //   ld a, e
            context->cpu->addLdAE();
            //   and 0x0F
            context->cpu->addAnd(0x0F);
            //   ld e, a
            context->cpu->addLdEA();
            // pop af
            context->cpu->addPopAF();

            // call XBASIC_USING_DO    ; a=format, d=thousand digits, e=decimal
            // digits, b:hl=number, out hl=string
            context->cpu->addCall(def_XBASIC_USING_DO);
            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "INSTR") {
          if (result[0] == Lexeme::subtype_string &&
              result[1] == Lexeme::subtype_string) {
            // ex de,hl        ; search string
            context->cpu->addExDEHL();
            // pop hl          ; source string
            context->cpu->addPopHL();

            // ld a, 0x01      ; search start
            context->cpu->addLdA(0x01);

            // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source,
            // de=search; out: hl=position)
            context->cpu->addCall(def_XBASIC_INSTR);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;

      case 3: {
        if (lexeme->value == "MID$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          // ld a, l             ; number of chars
          context->cpu->addLdAL();

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // pop bc
            context->cpu->addPopBC();
            // pop hl
            context->cpu->addPopHL();
            // push af
            context->cpu->addPushAF();
            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            // pop af
            context->cpu->addPopAF();
            result[1] = Lexeme::subtype_numeric;
          } else {
            // pop hl
            context->cpu->addPopHL();
          }

          if (result[2] == Lexeme::subtype_string &&
              result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // ld b, l         ; start char
            context->cpu->addLdBL();
            // pop hl          ; source string
            context->cpu->addPopHL();

            // call 0x7db1     ; xbasic mid string (in: b=start, a=size,
            // hl=source; out: hl=BUF)
            context->cpu->addCall(def_XBASIC_MID);

            // ld de, temporary string
            context->variableEmitter->addTempStr(false);
            // push de
            context->cpu->addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            context->cpu->addCall(def_XBASIC_COPY_STRING);
            // pop hl
            context->cpu->addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "INSTR") {
          // ex de,hl        ; search string
          context->cpu->addExDEHL();
          // pop hl          ; source string
          context->cpu->addPopHL();
          // pop bc          ; search start
          context->cpu->addPopBC();

          if (result[2] == Lexeme::subtype_single_decimal ||
              result[2] == Lexeme::subtype_double_decimal) {
            // ex (sp),hl
            context->cpu->addExiSPHL();
            // push de
            context->cpu->addPushDE();
            //   cast
            addCast(result[2], Lexeme::subtype_numeric);
            //   ld c, l
            context->cpu->addLdCL();
            // pop de
            context->cpu->addPopDE();
            // pop hl
            context->cpu->addPopHL();
            result[2] = Lexeme::subtype_numeric;
          }

          // ld a, c             ; search start
          context->cpu->addLdAC();

          if (result[0] == Lexeme::subtype_string &&
              result[1] == Lexeme::subtype_string &&
              result[2] == Lexeme::subtype_numeric) {
            // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source,
            // de=search; out: hl=position)
            context->cpu->addCall(def_XBASIC_INSTR);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;
    }
  }

  return result[0];
}
