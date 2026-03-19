#include "compiler_swap_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

void CompilerSwapStatementStrategy::cmd_swap(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;
  auto& variable = *context->variableEmitter;
  shared_ptr<Lexeme> lexeme1, lexeme2;
  shared_ptr<ActionNode> action1, action2;
  unsigned int t = context->current_action->actions.size();

  if (t == 2) {
    action1 = context->current_action->actions[0];
    lexeme1 = action1->lexeme;
    if (lexeme1->type != Lexeme::type_identifier) {
      context->syntaxError("Invalid parameter type in SWAP (1st)");
      return;
    }

    action2 = context->current_action->actions[1];
    lexeme2 = action2->lexeme;
    if (lexeme2->type != Lexeme::type_identifier) {
      context->syntaxError("Invalid parameter type in SWAP (2nd)");
      return;
    }

    if (lexeme1->subtype == lexeme2->subtype) {
      // ld hl, variable
      variable.addVarAddress(action1);
      // push hl
      cpu.addPushHL();
      // ld hl, variable
      variable.addVarAddress(action2);
      // pop de
      optimizer.addByteOptimized(0xD1);

      if (lexeme1->subtype == Lexeme::subtype_numeric) {
        // call 0x6bf5    ; xbasic SWAP integers (in: hl=var1, de=var2)
        cpu.addCall(def_XBASIC_SWAP_INTEGER);
      } else if (lexeme1->subtype == Lexeme::subtype_string) {
        // call 0x6bf9    ; xbasic SWAP strings (in: hl=var1, de=var2)
        cpu.addCall(def_XBASIC_SWAP_STRING);
      } else {
        // call 0x6bfd    ; xbasic SWAP floats (in: hl=var1, de=var2)
        cpu.addCall(def_XBASIC_SWAP_FLOAT);
      }

    } else {
      context->syntaxError("Parameters type mismatch in SWAP");
    }

  } else {
    context->syntaxError("Invalid SWAP parameters");
  }
}

bool CompilerSwapStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_swap(context);
  return context->compiled;
}
