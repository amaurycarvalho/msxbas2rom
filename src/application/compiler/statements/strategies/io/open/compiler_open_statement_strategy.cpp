#include "compiler_open_statement_strategy.h"

#include <cctype>

#include "action_node.h"
#include "build_options.h"
#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerOpenStatementStrategy::cmd_open(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;
  unsigned char direction = 0x04;  // RANDOM (default)

  context->file_support = true;

  if (t < 2 || t > 4) {
    context->syntaxError("Invalid OPEN parameters count");
    return;
  }

  // filename
  action = context->current_action->actions[0];
  result_subtype = expression.evalExpression(action);
  if (result_subtype != Lexeme::subtype_string) {
    context->syntaxError("OPEN filename must be a string");
    return;
  }
  // keep filename pointer
  cpu.addPushHL();

  // optional direction
  if (t == 3 || t == 4) {
    action = context->current_action->actions[1];
    lexeme = action->lexeme;
    if (!lexeme) {
      context->syntaxError("Invalid OPEN direction");
      return;
    }
    if (lexeme->value == "INPUT") {
      direction = 0x01;
    } else if (lexeme->value == "OUTPUT" || lexeme->value == "OUT") {
      direction = 0x02;
    } else if (lexeme->value == "APPEND" || lexeme->value == "APP") {
      direction = 0x08;
    } else {
      context->syntaxError("Invalid OPEN direction");
      return;
    }
  }

  // file number (always present)
  action = context->current_action->actions[(t == 2) ? 1 : 2];
  result_subtype = expression.evalExpression(action);
  expression.addCast(result_subtype, Lexeme::subtype_numeric);
  // a = file number
  cpu.addLdAL();

  // optional LEN
  if (t == 4) {
    cpu.addPushAF();
    action = context->current_action->actions[3];
    result_subtype = expression.evalExpression(action);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);
    cpu.addLdCL();
    cpu.addLdBH();
    cpu.addPopAF();
  } else {
    // default record length
    cpu.addLdBC(256);
  }

  // hl = filename string
  cpu.addPopHL();
  // e = direction
  cpu.addLdDE(direction);
  // bc = record length
  // a = file number
  // call cmd_fopen
  cpu.addCall(def_cmd_fopen);
}

bool CompilerOpenStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_open(context);
  return context->compiled;
}
