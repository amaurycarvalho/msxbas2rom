#include "compiler_print_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerPrintStatementStrategy::cmd_print(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme, last_lexeme = 0;
  shared_ptr<ActionNode> action, subaction;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool redirected = false;
  shared_ptr<FixNode> skipPrintMark;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      last_lexeme = lexeme;

      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            cpu.addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            if (context->has_open_grp) continue;

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
            // jp nz, skip PRINT statement
            if (!skipPrintMark) skipPrintMark = fixup.addMark();
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
            context->syntaxError("Invalid PRINT parameter separator");
            return;
          }
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
            context->syntaxError("Invalid PRINT parameter");
            return;
          }
        }
      }
    }

  } else {
    cpu.addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
  }

  if (last_lexeme) {
    if (last_lexeme->type != Lexeme::type_separator ||
        (last_lexeme->value != ";" && last_lexeme->value != ",")) {
      cpu.addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
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

  if (skipPrintMark) skipPrintMark->aimHere();
}

bool CompilerPrintStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_print(context);
  return context->compiled;
}
