#include "compiler_gosub_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "lexeme.h"

void CompilerGosubStatementStrategy::cmd_gosub(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  shared_ptr<Lexeme> lexeme;

  if (context->current_action->actions.size() == 1) {
    lexeme = context->current_action->actions[0].get()->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        // call address
        fixup.addFix(lexeme->value);
        cpu.addCall(0x0000);
        return;
      }
    }
  }

  context->syntaxError("Invalid GOSUB parameters");
}

bool CompilerGosubStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_gosub(context);
  return context->compiled;
}
