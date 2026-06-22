#include "compiler_input_statement_strategy.h"

#include <vector>

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "fix_node.h"
#include "lexeme.h"

bool CompilerInputStatementStrategy::isFileInput(
    shared_ptr<CompilerContext> context) {
  if (context->current_action->actions.empty()) return false;

  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<Lexeme> lexeme = action->lexeme;

  return lexeme && lexeme->type == Lexeme::type_separator &&
         lexeme->value == "#";
}

void CompilerInputStatementStrategy::cmd_file_input(
    shared_ptr<CompilerContext> context, bool lineMode) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  auto& variable = *context->variableEmitter;
  shared_ptr<ActionNode> action, subaction;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<FixNode> skipInputMark;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  action = context->current_action->actions[0];
  if (action->actions.empty()) {
    context->syntaxError("Invalid INPUT# file number");
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
  skipInputMark = fixup.addMark();
  cpu.addJpNZ(0x0000);  // skip INPUT# when disk is unavailable

  std::vector<shared_ptr<ActionNode>> values;
  bool expectValue = true;

  for (i = 1; i < t; i++) {
    action = context->current_action->actions[i];
    lexeme = action->lexeme;

    if (lexeme && lexeme->type == Lexeme::type_separator) {
      if (lexeme->value == ",") {
        if (expectValue) {
          context->syntaxError("Invalid INPUT# parameter separator");
          return;
        }
        expectValue = true;
        continue;
      }

      context->syntaxError("Invalid INPUT# parameter separator");
      return;
    }

    if (!expectValue) {
      context->syntaxError("Invalid INPUT# parameter separator");
      return;
    }

    if (!lexeme || lexeme->type != Lexeme::type_identifier) {
      context->syntaxError("Invalid INPUT# parameter type");
      return;
    }

    values.push_back(action);
    expectValue = false;
  }

  if (values.empty() || expectValue) {
    context->syntaxError("INPUT# with empty parameters");
    return;
  }

  for (i = 0; i < values.size(); i++) {
    lexeme = values[i]->lexeme;
    context->codeOptimizer->addKernelCall(DISP_GET_NEXT_TEMP_STRING_ADDRESS);
    cpu.addLdDE(lineMode ? 0x0001 : 0x0000);  // e=read mode for cmd_finput
    cpu.addPopAF();
    cpu.addPushAF();
    context->codeOptimizer->addKernelCall(DISP_cmd_finput);

    expression.addCast(Lexeme::subtype_string, lexeme->subtype);
    if (!variable.addAssignment(values[i])) return;
  }

  cpu.addPopAF();
  skipInputMark->aimHere();
}

void CompilerInputStatementStrategy::cmd_normal_input(
    shared_ptr<CompilerContext> context, bool questionMark) {
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            context->codeOptimizer->addKernelCall(
                DISP_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else {
            context->syntaxError("Invalid INPUT parameter separator");
            return;
          }
        } else {
          if (lexeme->type == Lexeme::type_identifier) {
            // choose between INPUT or LINE INPUT

            if (questionMark) {
              context->codeOptimizer->addKernelCall(DISP_XBASIC_INPUT_1);
            } else {
              context->codeOptimizer->addKernelCall(DISP_XBASIC_INPUT_2);
            }

            // do assignment

            expression.addCast(Lexeme::subtype_string, lexeme->subtype);

            if (!context->variableEmitter->addAssignment(action)) return;

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
              context->syntaxError("Invalid INPUT parameter");
              return;
            }
          }
        }
      }
    }

  } else {
    context->syntaxError();
  }
}

bool CompilerInputStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  if (isFileInput(context)) {
    cmd_file_input(context, false);
    return context->compiled;
  }

  cmd_normal_input(context, true);
  return context->compiled;
}

bool CompilerInputStatementStrategy::executeLineInput(
    shared_ptr<CompilerContext> context) {
  if (isFileInput(context)) {
    cmd_file_input(context, true);
    return context->compiled;
  }

  cmd_normal_input(context, false);
  return context->compiled;
}
