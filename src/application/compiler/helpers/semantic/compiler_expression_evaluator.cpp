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
#include "compiler_function_strategy_factory.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "logger.h"

CompilerExpressionEvaluator::CompilerExpressionEvaluator(
    CompilerContext* context)
    : context(context) {
  functionFactory.reset(new CompilerFunctionStrategyFactory());
}

CompilerExpressionEvaluator::~CompilerExpressionEvaluator() = default;

int CompilerExpressionEvaluator::evalExpression(ActionNode* action) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& variable = *context->variableEmitter;
  auto& floatConverter = *context->floatConverter;
  int result = Lexeme::subtype_unknown;
  shared_ptr<Lexeme> lexeme;

  lexeme = action->lexeme;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_identifier) {
      result = lexeme->subtype;

      if (lexeme->isArray || result == Lexeme::subtype_string) {
        if (!variable.addVarAddress(action)) {
          if (!lexeme->isArray && action->actions.size()) {
            context->syntaxError("Undeclared array or unknown function");
          } else
            result = Lexeme::subtype_unknown;
        } else {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            // ld e, (hl)
            cpu.addLdEiHL();
            // inc hl
            cpu.addIncHL();
            // ld d, (hl)
            cpu.addLdDiHL();
            // ex de, hl
            cpu.addExDEHL();
          } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                     lexeme->subtype == Lexeme::subtype_double_decimal) {
            // ld b, (hl)
            cpu.addLdBiHL();
            // inc hl
            cpu.addIncHL();
            // ld e, (hl)
            cpu.addLdEiHL();
            // inc hl
            cpu.addIncHL();
            // ld d, (hl)
            cpu.addLdDiHL();
            // ex de, hl
            cpu.addExDEHL();
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
          fixup.addFix(lexeme);
          cpu.addLdHLii(0x0000);
        } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                   lexeme->subtype == Lexeme::subtype_double_decimal) {
          // ld a, (variable)
          fixup.addFix(lexeme);
          cpu.addLdAii(0x0000);
          // ld b, a
          cpu.addLdBA();
          // ld hl, (variable+1)
          fixup.addFix(lexeme)->step = 1;
          cpu.addLdHLii(0x0000);
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
        fixup.addFix(lexeme);

        cpu.addLdHL(0x0000);  // ld hl, string

      } else if (lexeme->subtype == Lexeme::subtype_numeric) {
        char* s = (char*)lexeme->value.c_str();
        try {
          if (s[0] == '&') {
            if (s[1] == 'h' || s[1] == 'H')
              cpu.addLdHL(
                  stoi(lexeme->value.substr(2), 0, 16));  // ld hl, value
            else if (s[1] == 'o' || s[1] == 'O')
              cpu.addLdHL(stoi(lexeme->value.substr(2), 0, 8));  // ld hl, value
            else if (s[1] == 'b' || s[1] == 'B')
              cpu.addLdHL(stoi(lexeme->value.substr(2), 0, 2));  // ld hl, value
            else
              result = Lexeme::subtype_unknown;
          } else
            cpu.addLdHL(stoi(lexeme->value));  // ld hl, value
        } catch (exception& e) {
          context->logger->warning("Error while converting numeric constant " +
                                   lexeme->value);
          cpu.addLdHL(0x0000);  // ld hl, value
        }

      } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                 lexeme->subtype == Lexeme::subtype_double_decimal) {
        int value = floatConverter.str2FloatLib(lexeme->value);

        cpu.addLdB((value >> 16) & 0xff);  // ld b, value
        cpu.addLdHL(value & 0xffff);       // ld hl, value

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
  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& optimizer = *context->codeOptimizer;
  int result = Lexeme::subtype_unknown;
  shared_ptr<Lexeme> lexeme;
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
          optimizer.addKernelCall(def_intCompareNOT);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "-") {
        if (result == Lexeme::subtype_numeric) {
          // call intNEG
          // ctx.cpu->addCall(def_intNEG);
          optimizer.addKernelCall(def_intNEG);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // call floatNeg
          // ctx.cpu->addCall(def_floatNEG);
          optimizer.addKernelCall(def_floatNEG);

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
          optimizer.addByteOptimized(0xD1);

          // call intCompareAND
          // ctx.cpu->addCall(def_intCompareAND);
          optimizer.addKernelCall(def_intCompareAND);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "OR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareOR
          // ctx.cpu->addCall(def_intCompareOR);
          optimizer.addKernelCall(def_intCompareOR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "XOR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareXOR
          // ctx.cpu->addCall(def_intCompareXOR);
          optimizer.addKernelCall(def_intCompareXOR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "EQV") {
        // same as: not a xor b

        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // ld a, l
          cpu.addLdAL();
          // xor e
          cpu.addXorE();
          // cpl
          cpu.addCPL();
          // ld l, a
          cpu.addLdLA();
          // ld a, h
          cpu.addAddH();
          // xor d
          cpu.addXorD();
          // cpl
          cpu.addCPL();
          // ld h, a
          cpu.addLdHA();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "IMP") {
        if (result == Lexeme::subtype_numeric) {
          // same as: not a or b

          // pop de
          optimizer.addByteOptimized(0xD1);

          // ld a, e
          cpu.addLdAE();
          // cpl
          cpu.addCPL();
          // or l
          cpu.addOrL();
          // ld l, a
          cpu.addLdLA();
          // ld a, d
          cpu.addLdAD();
          // cpl
          cpu.addCPL();
          // or h
          cpu.addOrH();
          // ld h, a
          cpu.addLdHA();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareEQ
          // ctx.cpu->addCall(def_intCompareEQ);
          optimizer.addKernelCall(def_intCompareEQ);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();

          // 7876 xbasic compare floats (=)
          cpu.addCall(def_XBASIC_COMPARE_FLOATS_EQ);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          cpu.addPopHL();

          // 7eae xbasic compare string (NULBUF = string)
          cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<>") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareNE
          // ctx.cpu->addCall(def_intCompareNE);
          optimizer.addKernelCall(def_intCompareNE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();

          // 787f xbasic compare floats (<>)
          cpu.addCall(def_XBASIC_COMPARE_FLOATS_NE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          cpu.addPopHL();

          // 7ec9 xbasic compare string (NULBUF <> string)
          cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_NE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareLT
          // ctx.cpu->addCall(def_intCompareLT);
          optimizer.addKernelCall(def_intCompareLT);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();

          // 7888 xbasic compare floats (>)
          cpu.addCall(def_XBASIC_COMPARE_FLOATS_GT);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          cpu.addPopHL();

          // 7ea4 xbasic compare string (NULBUF > string)
          cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_GT);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareLE
          // ctx.cpu->addCall(def_intCompareLE);
          optimizer.addKernelCall(def_intCompareLE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();

          // 7892 xbasic compare floats (>=)
          cpu.addCall(def_XBASIC_COMPARE_FLOATS_GE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          cpu.addPopHL();

          // 7eb7 xbasic compare string (NULBUF >= string)
          cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_GE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == ">") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareGT
          // ctx.cpu->addCall(def_intCompareGT);
          optimizer.addKernelCall(def_intCompareGT);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();

          // 789b xbasic compare floats (<)
          cpu.addCall(def_XBASIC_COMPARE_FLOATS_LT);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          cpu.addPopHL();

          // 7ec0 xbasic compare string (NULBUF < string)
          cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_LT);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == ">=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intCompareGE
          // ctx.cpu->addCall(def_intCompareGE);
          optimizer.addKernelCall(def_intCompareGE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();

          // 78a4 xbasic compare floats (<=)
          cpu.addCall(def_XBASIC_COMPARE_FLOATS_LE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          cpu.addPopHL();

          // 7ed2 xbasic compare string (NULBUF <= string)
          cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_LE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "+") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);
          // add hl, de      ; add integers (math optimized)
          optimizer.addByteOptimized(0x19);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();
          // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
          cpu.addCall(def_XBASIC_ADD_FLOATS);

        } else if (result == Lexeme::subtype_string) {
          // pop bc                      ; bc=string 1, hl=string 2
          cpu.addPopBC();
          // ld de, temporary variable   ; de=string destination
          variable.addTempStr(false);
          // push de
          cpu.addPushDE();
          //   call 0x7f05               ; xbasic concat strings (in: bc=str1,
          //   hl=str2, de=strdest; out: hl fake)
          cpu.addCall(def_XBASIC_CONCAT_STRINGS);
          // pop hl                      ; correct destination
          cpu.addPopHL();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "-") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);
          // ex de,hl
          optimizer.addByteOptimized(0xEB);

          s = cpu.context->code_pipeline[0];
          i = s[1] | (s[2] << 8);
          if (s[0] == 0x11 && i <= 4) {  // ld de, n
            cpu.context->code_pointer -= 3;
            cpu.context->code_size -= 3;
            while (i) {
              // dec hl
              cpu.addDecHL();
              i--;
            }
          } else {
            // and a
            cpu.addAndA();
            // sbc hl, de      ; subtract integers
            cpu.addSbcHLDE();
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ex de,hl
          cpu.addExDEHL();
          // ld c, b
          cpu.addLdCB();
          // ld b, a
          cpu.addLdBA();
          // call 0x76bd     ; subtract floats (b:hl - c:de = b:hl)
          cpu.addCall(def_XBASIC_SUBTRACT_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "*") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          /// @remark math optimization when second parameter is a integer
          /// constant
          if (context->opts->megaROM)
            s = cpu.context->code_pipeline[1];
          else
            s = cpu.context->code_pipeline[0];

          i = s[1] | (s[2] << 8);

          if (action->actions[0]->lexeme->type == Lexeme::type_literal &&
              i <= 256) {
            if (context->opts->megaROM) {
              cpu.context->code_pointer -= 5;  //! @todo verify if 5 or 6
              cpu.context->code_size -= 5;
            } else {
              cpu.context->code_pointer -= 4;
              cpu.context->code_size -= 4;

              if (action->actions[1]->lexeme->type == Lexeme::type_literal) {
                cpu.context->code_pointer += 1;
                cpu.context->code_size += 1;
                s = &cpu.context->code[cpu.context->code_pointer - 3];
                if (s[0] == 0x11) s[0] = 0x21;  // change "ld de,n" to "ld hl,n"
              }
            }

            switch (i) {
              case 0: {
                cpu.addLdHL(0x0000);
              } break;

              case 128: {
                // XOR A | SRL H | RR L | RRA | LD H, L | LD L, A
                cpu.addXorA();
                cpu.addSRLH();
                cpu.addRRL();
                cpu.addRRA();
                cpu.addLdHL();
                cpu.addLdLA();
              } break;

              case 256: {
                //  LD H, L | LD L, 0
                cpu.addLdHL();
                cpu.addLdL(0);
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
                  cpu.addLdEL();
                  cpu.addLdDH();
                }
                while (n) {
                  n--;
                  cpu.addAddHLHL();
                  if (b[n]) {
                    cpu.addAddHLDE();
                  }
                }
              } break;
            }

          } else {
            // call 0x761b     ; multiply integers (hl = hl * de)
            cpu.addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ld c, a
          cpu.addLdCA();
          // call 0x7732     ; multiply floats
          cpu.addCall(def_XBASIC_MULTIPLY_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "/") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);
          // ex de,hl
          optimizer.addByteOptimized(0xEB);

          // math optimization when second parameter is a integer constant
          s = cpu.context->code_pipeline[0];
          if (s[0] == 0x11) {  // ld de, n
            i = s[1] | (s[2] << 8);

            switch (i) {
              case 0: {
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                cpu.addLdHL(0x0000);
              } break;

              case 1: {
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
              } break;

              case 2: {
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // sra h | rr l
                cpu.addSRAH();
                cpu.addRRL();
                // JRNC $+6   ; jump if there's no rest of division by 2
                cpu.addJrNC(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 4: {
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD A, L
                cpu.addLdAL();
                // sra h | rr l (2 times)
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                // and 0x03
                cpu.addAnd(0x03);
                // JRZ $+6   ; jump if there's no rest of division by 4
                cpu.addJrZ(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 8: {
                // OLD: LD A, L | SRA H | RRA | SRL H | RRA | SRL H | RRA | LD
                // L, A
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD A, L
                cpu.addLdAL();
                // sra h | rr l (3 times)
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                // and 0x07
                cpu.addAnd(0x07);
                // JRZ $+6   ; jump if there's no rest of division by 8
                cpu.addJrZ(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 16: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL
                // | RLA | ADD HL, HL | RLA | LD L, H | LD H, A
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD A, L
                cpu.addLdAL();
                // sra h | rr l (4 times)
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                // and 0x0F
                cpu.addAnd(0x0F);
                // JRZ $+6   ; jump if there's no rest of division by 16
                cpu.addJrZ(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 32: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL
                // | RLA | LD L, H | LD H, A
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD A, L
                cpu.addLdAL();
                // sra h | rr l (5 times)
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                // and 0x1F
                cpu.addAnd(0x1F);
                // JRZ $+6   ; jump if there's no rest of division by 32
                cpu.addJrZ(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 64: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | LD L, H |
                // LD H, A
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD A, L
                cpu.addLdAL();
                // sra h | rr l (6 times)
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                // and 0x3F
                cpu.addAnd(0x3F);
                // JRZ $+6   ; jump if there's no rest of division by 64
                cpu.addJrZ(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 128: {
                // old: XOR A | ADD HL, HL | RLA | LD L, H | LD H, A
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD A, L
                cpu.addLdAL();
                // sra h | rr l (7 times)
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                cpu.addSRAH();
                cpu.addRRL();
                // and 0x7F
                cpu.addAnd(0x7F);
                // JRZ $+6   ; jump if there's no rest of division by 64
                cpu.addJrZ(0x06);
                //   LD A, H
                cpu.addLdAH();
                //   AND 0x80  ; sign bit
                cpu.addAnd(0x80);
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              case 256: {
                cpu.context->code_pointer -= 3;
                cpu.context->code_size -= 3;
                // LD E, L
                cpu.addLdEL();
                // LD L, H
                cpu.addLdLH();
                // LD H, 0x00
                cpu.addLdH(0x00);
                // LD A, L
                cpu.addLdAL();
                // AND 0x80    ; sign bit
                cpu.addAnd(0x80);
                // JRZ $+7
                cpu.addJrZ(0x07);
                //   LD H, 0xFF
                cpu.addLdH(0xFF);
                //   XOR A
                cpu.addXorA();
                //   OR E
                cpu.addOrE();
                //   JRZ $+1
                cpu.addJrZ(0x01);
                //     INC HL
                cpu.addIncHL();
              } break;

              default: {
                // call divide integers
                cpu.addCall(def_XBASIC_DIVIDE_INTEGERS);
              } break;
            }

          } else {
            // call divide integers
            cpu.addCall(def_XBASIC_DIVIDE_INTEGERS);
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ex de,hl
          cpu.addExDEHL();
          // ld c, b
          cpu.addLdCB();
          // ld b, a
          cpu.addLdBA();
          // call 0x7775     ; divide floats
          cpu.addCall(def_XBASIC_DIVIDE_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "\\") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);
          // ex de,hl
          optimizer.addByteOptimized(0xEB);
          // call 0x762d     ; divide integers
          cpu.addCall(def_XBASIC_DIVIDE_INTEGERS);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop bc
          cpu.addPopBC();
          // pop de
          cpu.addPopDE();
          // push hl
          cpu.addPushHL();
          // ex de, hl
          cpu.addExDEHL();

          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop de
          cpu.addPopDE();

          // call 0x762d     ; divide integers
          cpu.addCall(def_XBASIC_DIVIDE_INTEGERS);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "^") {
        if (result == Lexeme::subtype_numeric) {
          // ex de,hl
          optimizer.addByteOptimized(0xEB);

          // pop hl
          cpu.addPopHL();

          //   call 0x782D     ; integer to float
          cpu.addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT);

          // call 0x77C1       ; power float ^ integer
          cpu.addCall(def_XBASIC_POWER_FLOAT_TO_INTEGER);

          result = Lexeme::subtype_single_decimal;

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          cpu.addPopAF();
          // pop de
          cpu.addPopDE();
          // ex de,hl
          cpu.addExDEHL();
          // ld c, b
          cpu.addLdCB();
          // ld b, a
          cpu.addLdBA();
          // call 0x780d      ; power float ^ float
          cpu.addCall(def_XBASIC_POWER_FLOAT_TO_FLOAT);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "MOD") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);
          // ex de,hl
          optimizer.addByteOptimized(0xEB);
          // call 0x762d     ; divide integers
          cpu.addCall(def_XBASIC_DIVIDE_INTEGERS);
          // ex de, hl       ; remainder
          cpu.addExDEHL();

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop bc
          cpu.addPopBC();
          // pop de
          cpu.addPopDE();
          // push hl
          cpu.addPushHL();
          // ex de, hl
          cpu.addExDEHL();

          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop de
          cpu.addPopDE();

          // call 0x762d     ; divide integers
          cpu.addCall(def_XBASIC_DIVIDE_INTEGERS);

          // ex de, hl       ; remainder
          cpu.addExDEHL();

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "SHR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intSHR
          // ctx.cpu->addCall(def_intSHR);
          optimizer.addKernelCall(def_intSHR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "SHL") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          optimizer.addByteOptimized(0xD1);

          // call intSHL
          // ctx.cpu->addCall(def_intSHL);
          optimizer.addKernelCall(def_intSHL);

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
  auto& cpu = *context->cpu;
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
        cpu.addPushHL();
        if (subtype == Lexeme::subtype_single_decimal ||
            subtype == Lexeme::subtype_double_decimal) {
          // push bc
          cpu.addPushBC();
        }
      }
    }
  }

  return result;
}

int CompilerExpressionEvaluator::evalOperatorCast(ActionNode* action) {
  auto& cpu = *context->cpu;
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
    cpu.addCall(def_castParamFloatInt);

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
    cpu.addPopDE();
    // push hl
    cpu.addPushHL();
    // ex de,hl
    cpu.addExDEHL();

    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    // pop de   ; swap again
    cpu.addPopDE();
    // push hl
    cpu.addPushHL();
    // ex de,hl
    cpu.addExDEHL();

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
  auto& cpu = *context->cpu;
  if (from != to) {
    if (from == Lexeme::subtype_numeric) {
      if (to == Lexeme::subtype_numeric) {
        return;
      } else if (to == Lexeme::subtype_string) {
        // call 0x7b26   ; xbasic int to string (in hl, out hl)
        cpu.addCall(def_XBASIC_CAST_INTEGER_TO_STRING);
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        // call 0x782d   ; xbasic int to float (in hl, out b:hl)
        cpu.addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT);
      }

    } else if (from == Lexeme::subtype_string) {
      if (to == Lexeme::subtype_numeric) {
        // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
        // b:hl)
        cpu.addCall(def_XBASIC_CAST_STRING_TO_FLOAT);
        // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
        cpu.addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER);
      } else if (to == Lexeme::subtype_string) {
        return;
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
        // b:hl)
        cpu.addCall(def_XBASIC_CAST_STRING_TO_FLOAT);
      }

    } else if (from == Lexeme::subtype_single_decimal ||
               from == Lexeme::subtype_double_decimal) {
      if (to == Lexeme::subtype_numeric) {
        // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
        cpu.addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER);
      } else if (to == Lexeme::subtype_string) {
        // call 0x7b80   ; xbasic float to string (in b:hl, out hl)
        cpu.addCall(def_XBASIC_CAST_FLOAT_TO_STRING);
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
  shared_ptr<Lexeme> lexeme;
  ActionNode* next_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < 4; i++) result[i] = Lexeme::subtype_unknown;

  lexeme = action->lexeme;

  if (lexeme) {
    ICompilerFunctionStrategy* strategy =
        functionFactory->getByKeyword(lexeme->value);
    if (strategy && (lexeme->value == "VARPTR" || lexeme->value == "USING$")) {
      return strategy->execute(context, action, result, t);
    }

    if (t) {
      if (!evalOperatorParms(action, t)) return result[0];

      for (i = 0; i < t; i++) {
        next_action = action->actions[i];
        result[i] = next_action->subtype;

        if (result[i] == Lexeme::subtype_unknown) return result[i];
      }
    }
    if (strategy) {
      int strategyResult = strategy->execute(context, action, result, t);
      if (strategyResult != Lexeme::subtype_unknown) return strategyResult;
    }

    result[0] = Lexeme::subtype_unknown;
  }

  return result[0];
}
