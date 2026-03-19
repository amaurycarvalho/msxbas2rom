#include "compiler_for_statement_strategy.h"

#include <memory>

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "compiler_symbol_resolver.h"
#include "compiler_variable_emitter.h"
#include "fix_node.h"
#include "lexeme.h"

using namespace std;

void CompilerForStatementStrategy::cmd_for(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  auto& evaluator = *context->evaluator;
  shared_ptr<Lexeme> lexeme, lex_var = nullptr;
  shared_ptr<ActionNode> action, var_action;
  shared_ptr<ActionNode> saved_action = context->current_action;
  unsigned int i, t = saved_action->actions.size();
  int result_subtype;
  shared_ptr<ForNextNode> forNext;
  bool has_let = false, has_to = false, has_step = false;

  if (!t) {
    context->syntaxError("FOR parameters is missing");
  } else {
    context->for_count++;

    forNext = make_shared<ForNextNode>();
    context->forNextStack.push(forNext);

    forNext->index = context->for_count;
    forNext->tag = context->current_tag;
    forNext->for_to =
        make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_numeric,
                            "FOR_TO_" + to_string(context->for_count));
    forNext->for_to_action = make_shared<ActionNode>();
    forNext->for_to_action->lexeme = forNext->for_to;
    context->symbolResolver->addSymbol(forNext->for_to);

    forNext->for_step =
        make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_numeric,
                            "FOR_STEP_" + to_string(context->for_count));
    forNext->for_step_action = make_shared<ActionNode>();
    forNext->for_step_action->lexeme = forNext->for_step;
    context->symbolResolver->addSymbol(forNext->for_step);

    for (i = 0; i < t; i++) {
      action = saved_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_keyword) {
        if (lexeme->value == "LET") {
          if (action->actions.size()) {
            var_action = action->actions[0];
            lex_var = var_action->lexeme;
            forNext->for_var = lex_var;
            forNext->for_to->subtype = lex_var->subtype;
            forNext->for_step->subtype = lex_var->subtype;
          } else {
            context->syntaxError(
                "Invalid FOR expression (variable assignment)");
            return;
          }

          if (lex_var->type != Lexeme::type_identifier) {
            context->syntaxError(
                "Invalid FOR expression (variable is missing)");
            return;
          } else {
            if (lex_var->subtype != Lexeme::subtype_numeric &&
                lex_var->subtype != Lexeme::subtype_single_decimal &&
                lex_var->subtype != Lexeme::subtype_double_decimal) {
              context->syntaxError("Invalid FOR expression (wrong data type)");
              return;
            }
          }

          if (!evaluator.evalAction(action)) return;

          has_let = true;

        } else if (lexeme->value == "TO") {
          // ld hl, data parameter

          result_subtype = expression.evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric ||
              result_subtype == Lexeme::subtype_single_decimal ||
              result_subtype == Lexeme::subtype_double_decimal) {
            expression.addCast(result_subtype, lex_var->subtype);

            context->variableEmitter->addAssignment(forNext->for_to_action);

            has_to = true;

          } else {
            context->syntaxError("Invalid TO expression (wrong data type)");
            return;
          }

        } else if (lexeme->value == "STEP") {
          // ld hl, data parameter

          result_subtype = expression.evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric ||
              result_subtype == Lexeme::subtype_single_decimal ||
              result_subtype == Lexeme::subtype_double_decimal) {
            expression.addCast(result_subtype, lex_var->subtype);

            context->variableEmitter->addAssignment(forNext->for_step_action);

            has_step = true;

          } else {
            context->syntaxError("Invalid STEP expression (wrong data type)");
            return;
          }

        } else {
          context->syntaxError("Invalid FOR syntax");
          return;
        }

      } else {
        context->syntaxError("Invalid FOR parameter type");
        return;
      }
    }

    if (has_let && has_to) {
      if (!has_step && lex_var->subtype != Lexeme::subtype_numeric) {
        // ld hl, 1
        cpu.addLdHL(0x0001);

        expression.addCast(Lexeme::subtype_numeric, forNext->for_step->subtype);

        context->variableEmitter->addAssignment(forNext->for_step_action);
      }

      if (lex_var->subtype == Lexeme::subtype_numeric) {
        // ld hl, (variable)
        fixup.addFix(forNext->for_var);
        cpu.addLdHLii(0x0000);

        // jr $+12      ; jump to check code
        if (has_step) {
          cpu.addJr(0x0B);
        } else {
          cpu.addJr(0x07);
        }

        // ;step code
        forNext->for_step_mark = fixup.addPreMark();
        forNext->for_step_mark->address = cpu.context->code_pointer;

        // ld hl, (variable)
        fixup.addFix(forNext->for_var);
        cpu.addLdHLii(0x0000);

        if (has_step) {
          // ld de, (step)
          cpu.addByte(0xED);
          fixup.addFix(forNext->for_step);
          cpu.addCmd(0x5B, 0x0000);
          // add hl,de
          cpu.addAddHLDE();
        } else {
          // inc hl
          cpu.addIncHL();
        }

        // ld (variable), hl
        fixup.addFix(forNext->for_var);
        cpu.addLdiiHL(0x0000);

        // ;check code

        // ex de, hl         ; after, de = (variable)
        cpu.addExDEHL();

        // ld hl, (to)
        fixup.addFix(forNext->for_to);
        cpu.addLdHLii(0x0000);

        if (has_step) {
          // ld a, (step+1)
          fixup.addFix(forNext->for_step)->step = 1;
          cpu.addLdAii(0x0000);

          // bit 7, a
          cpu.addWord(0xCB, 0x7F);
          // jr z, $+2
          cpu.addJrZ(0x01);
          //   ex de,hl
          cpu.addExDEHL();
        }

        // ;var > to? goto end for

        // call intCompareGT
        // cpuOpcodeWriter->addCall(def_intCompareGT);
        optimizer.addKernelCall(def_intCompareGT);

        // jp nz, end_for
        forNext->for_end_mark = fixup.addMark();
        cpu.addJpNZ(0x0000);

        // body start

      } else {
        // jr $+26      ; jump to check code
        cpu.addJr(0x19);

        // ;step code
        forNext->for_step_mark = fixup.addPreMark();
        forNext->for_step_mark->address = cpu.context->code_pointer;

        // ld a, (variable)
        fixup.addFix(forNext->for_var);
        cpu.addLdAii(0x0000);
        // ld b, a
        cpu.addLdBA();
        // ld hl, (variable+1)
        fixup.addFix(forNext->for_var)->step = 1;
        cpu.addLdHLii(0x0000);

        // ld a, (step)
        fixup.addFix(forNext->for_step);
        cpu.addLdAii(0x0000);
        // ld c, a
        cpu.addLdCA();
        // ld de, (step)
        cpu.addByte(0xED);
        fixup.addFix(forNext->for_step)->step = 1;
        cpu.addCmd(0x5B, 0x0000);
        // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
        cpu.addCall(def_XBASIC_ADD_FLOATS);

        // ld a, b
        cpu.addLdAB();
        // ld (variable), a
        fixup.addFix(forNext->for_var);
        cpu.addLdiiA(0x0000);
        // ld (variable+1), hl
        fixup.addFix(forNext->for_var)->step = 1;
        cpu.addLdiiHL(0x0000);

        // ;check code

        // ld a, (to)
        fixup.addFix(forNext->for_to);
        cpu.addLdAii(0x0000);
        // ld b, a
        cpu.addLdBA();
        // ld hl, (to+1)
        fixup.addFix(forNext->for_to)->step = 1;
        cpu.addLdHLii(0x0000);

        // ld a, (variable)
        fixup.addFix(forNext->for_var);
        cpu.addLdAii(0x0000);
        // ld c, a
        cpu.addLdCA();
        // ld de, (variable+1)
        cpu.addByte(0xED);
        fixup.addFix(forNext->for_var)->step = 1;
        cpu.addCmd(0x5B, 0x0000);

        // ld a, (step+2)
        fixup.addFix(forNext->for_step)->step = 2;
        cpu.addLdAii(0x0000);

        // bit 7, a
        cpu.addWord(0xCB, 0x7F);
        // jr nz, $+5
        cpu.addJrNZ(0x04);
        //   ex de,hl
        cpu.addExDEHL();
        //   ld a, c
        cpu.addLdAC();
        //   ld c, b
        cpu.addLdCB();
        //   ld b, a
        cpu.addLdBA();

        // ;var > to? goto end for

        // 78a4 xbasic compare floats (<=)
        cpu.addCall(def_XBASIC_COMPARE_FLOATS_LE);

        // ld a, l
        cpu.addLdAL();
        // or h
        cpu.addOrH();

        // jp z, end_for
        forNext->for_end_mark = fixup.addMark();
        cpu.addJpZ(0x0000);

        // body start
      }

    } else {
      context->syntaxError("Incomplete FOR syntax");
    }
  }
}

bool CompilerForStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_for(context);
  return context->compiled;
}
