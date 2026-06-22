#include "compiler_print_statement_strategy.h"

#include <vector>

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

bool CompilerPrintStatementStrategy::isFilePrint(
    shared_ptr<CompilerContext> context) {
  if (context->current_action->actions.empty()) return false;

  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<Lexeme> lexeme = action->lexeme;

  return lexeme && lexeme->type == Lexeme::type_separator &&
         lexeme->value == "#";
}

void CompilerPrintStatementStrategy::cmd_file_print(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, subaction;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<FixNode> skipPrintMark;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  action = context->current_action->actions[0];
  if (action->actions.empty()) {
    context->syntaxError("Invalid PRINT# file number");
    return;
  }

  subaction = action->actions[0];
  result_subtype = expression.evalExpression(subaction);
  expression.addCast(result_subtype, Lexeme::subtype_numeric);
  cpu.addLdAL();  // keep file number in A
  cpu.addPushAF();

  context->file_support = true;
  cpu.addLdA(0x00);  // drive A:
  context->codeOptimizer->addKernelCall(
      DISP_cmd_preflight_disk);  // check disk support
  cpu.addAndA();
  skipPrintMark = fixup.addMark();
  cpu.addJpNZ(0x0000);  // skip PRINT# when disk is unavailable

  std::vector<shared_ptr<ActionNode>> values;
  std::vector<string> separatorsAfterValue;
  bool expectValue = true;
  bool trailingSeparator = false;

  for (i = 1; i < t; i++) {
    action = context->current_action->actions[i];
    lexeme = action->lexeme;
    if (lexeme && lexeme->type == Lexeme::type_separator) {
      if (lexeme->value == "," || lexeme->value == ";") {
        if (expectValue) {
          context->syntaxError("Invalid PRINT# parameter separator");
          return;
        }
        separatorsAfterValue.push_back(lexeme->value);
        expectValue = true;
        trailingSeparator = true;
        continue;
      }
      context->syntaxError("Invalid PRINT# parameter separator");
      return;
    }

    if (!expectValue) {
      context->syntaxError("Invalid PRINT# parameter separator");
      return;
    }

    values.push_back(action);
    expectValue = false;
    trailingSeparator = false;
  }

  if (!expectValue && separatorsAfterValue.size() < values.size()) {
    separatorsAfterValue.push_back("");
  }

  if (values.empty()) {
    // PRINT #n : output only LF
    context->codeOptimizer->addKernelCall(DISP_GET_NEXT_TEMP_STRING_ADDRESS);
    cpu.addXorA();
    cpu.addLdiHLA();  // temporary string length = 0
    cpu.addPopAF();
    cpu.addPushAF();
    cpu.addLdDE(0x0A00);  // d=suffix2 LF, e=suffix1 0
    context->codeOptimizer->addKernelCall(DISP_cmd_fprint);
    cpu.addPopAF();
  } else {
    for (i = 0; i < values.size(); i++) {
      int suffix1 = 0x00;  // e
      int suffix2 = 0x00;  // d
      const string& separator = separatorsAfterValue[i];

      if (separator == ",") {
        suffix1 = 0x09;  // TAB
      } else if (i == values.size() - 1 && !trailingSeparator) {
        suffix1 = 0x0D;  // CR
        suffix2 = 0x0A;  // LF
      }

      result_subtype = expression.evalExpression(values[i]);
      expression.addCast(result_subtype, Lexeme::subtype_string);
      cpu.addPopAF();
      cpu.addPushAF();
      cpu.addLdDE((suffix2 << 8) | suffix1);  // d=suffix2, e=suffix1
      context->codeOptimizer->addKernelCall(DISP_cmd_fprint);
    }
    cpu.addPopAF();
  }

  skipPrintMark->aimHere();
}

void CompilerPrintStatementStrategy::cmd_normal_print(
    shared_ptr<CompilerContext> context) {
  // auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme, last_lexeme = 0;
  shared_ptr<ActionNode> action, subaction;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  // bool redirected = false;
  shared_ptr<FixNode> skipPrintMark;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      last_lexeme = lexeme;

      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            context->codeOptimizer->addKernelCall(
                DISP_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
            /*
          } else if (lexeme->value == "#") {
            if (context->has_open_grp) continue;

            subaction = action->actions[0];
            result_subtype = expression.evalExpression(subaction);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            context->file_support = true;
            // ld a, 0                ; drive A:
            cpu.addLdA(0x00);
            // call preflight disk
            context->codeOptimizer->addKernelCall(DISP_cmd_preflight_disk);
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
            */
          } else {
            context->syntaxError("Invalid PRINT parameter separator");
            return;
          }
        } else {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_string) {
            context->codeOptimizer->addKernelCall(
                DISP_XBASIC_PRINT_STR);  // call print_str

          } else if (result_subtype == Lexeme::subtype_numeric) {
            context->codeOptimizer->addKernelCall(
                DISP_XBASIC_PRINT_INT);  // call print_int

          } else if (result_subtype == Lexeme::subtype_single_decimal ||
                     result_subtype == Lexeme::subtype_double_decimal) {
            context->codeOptimizer->addKernelCall(
                DISP_XBASIC_PRINT_FLOAT);  // call print_float

          } else {
            context->syntaxError("Invalid PRINT parameter");
            return;
          }
        }
      }
    }

  } else {
    context->codeOptimizer->addKernelCall(
        DISP_XBASIC_PRINT_CRLF);  // call print_crlf
  }

  if (last_lexeme) {
    if (last_lexeme->type != Lexeme::type_separator ||
        (last_lexeme->value != ";" && last_lexeme->value != ",")) {
      context->codeOptimizer->addKernelCall(
          DISP_XBASIC_PRINT_CRLF);  // call print_crlf
    }
  }

  /*
  if (redirected) {
    // call io screen
    if (context->ioScreenMark)
      fixup.addFix(context->ioScreenMark->symbol);
    else
      context->ioScreenMark = fixup.addMark();
    cpu.addCall(0x0000);
  }
  */

  if (skipPrintMark) skipPrintMark->aimHere();
}

void CompilerPrintStatementStrategy::cmd_print(
    shared_ptr<CompilerContext> context) {
  if (isFilePrint(context)) {
    cmd_file_print(context);
    return;
  }

  cmd_normal_print(context);
}

bool CompilerPrintStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_print(context);
  return context->compiled;
}
