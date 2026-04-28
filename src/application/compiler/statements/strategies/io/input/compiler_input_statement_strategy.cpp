#include "compiler_input_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerInputStatementStrategy::cmd_input(
    shared_ptr<CompilerContext> context, bool questionMark) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action, subaction;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool redirected = false;
  shared_ptr<FixNode> skipInputMark;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            cpu.addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            subaction = action->actions[0];
            result_subtype = expression.evalExpression(subaction);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            context->file_support = true;
            // ld a, 0                ; drive A:
            cpu.addLdA(0x00);
            // call preflight disk
            cpu.addCall(def_cmd_preflight_disk);
            // and a
            cpu.addAndA();
            // jp nz, skip INPUT statement
            if (!skipInputMark) skipInputMark = fixup.addMark();
            cpu.addJpNZ(0x0000);

            redirected = true;
            // call io redirect
            if (context->ioRedirectMark)
              fixup.addFix(context->ioRedirectMark->symbol);
            else
              context->ioRedirectMark = fixup.addMark();
            cpu.addCall(0x0000);

            continue;
          } else {
            context->syntaxError("Invalid INPUT parameter separator");
            return;
          }
        } else {
          if (lexeme->type == Lexeme::type_identifier) {
            // choose between INPUT or LINE INPUT

            if (questionMark) {
              cpu.addCall(def_XBASIC_INPUT_1);
            } else {
              cpu.addCall(def_XBASIC_INPUT_2);
            }

            // do assignment

            expression.addCast(Lexeme::subtype_string, lexeme->subtype);

            if (!context->variableEmitter->addAssignment(action)) return;

          } else {
            result_subtype = expression.evalExpression(action);

            if (result_subtype == Lexeme::subtype_string) {
              cpu.addCall(def_XBASIC_PRINT_STR);  // call print_str

            } else if (result_subtype == Lexeme::subtype_numeric) {
              cpu.addCall(def_XBASIC_PRINT_INT);  // call print_int

            } else if (result_subtype == Lexeme::subtype_single_decimal ||
                       result_subtype == Lexeme::subtype_double_decimal) {
              cpu.addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

            } else {
              context->syntaxError("Invalid INPUT parameter");
              return;
            }
          }
        }
      }
    }

    if (redirected) {
      // call io screen
      if (context->ioScreenMark)
        fixup.addFix(context->ioScreenMark->symbol);
      else
        context->ioScreenMark = fixup.addMark();
      cpu.addCall(0x0000);
    }

    if (skipInputMark) skipInputMark->aimHere();

  } else {
    context->syntaxError();
  }
}

bool CompilerInputStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_input(context, true);
  return context->compiled;
}

bool CompilerInputStatementStrategy::executeLineInput(
    shared_ptr<CompilerContext> context) {
  cmd_input(context, false);
  return context->compiled;
}
