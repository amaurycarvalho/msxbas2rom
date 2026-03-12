#include "compiler_cmd_statement_strategy.h"

#include "compiler_cmd_handler_factory.h"
#include "compiler_context.h"

void CompilerCmdStatementStrategy::cmd_cmd(CompilerContext* context) {
  auto& actions = context->current_action->actions;
  ICompilerCmdHandler* handler;

  if (actions.empty()) {
    context->syntaxError("CMD with empty parameters");
    return;
  }

  for (auto action : actions) {
    handler = factory->getByKeyword(action->lexeme->value);
    if (!handler) {
      context->syntaxError("CMD statement invalid");
      return;
    }

    if (!handler->execute(context, action)) {
      if (!context->containErrors()) context->syntaxError();
      return;
    }
  }
}

CompilerCmdStatementStrategy::CompilerCmdStatementStrategy() {
  factory.reset(new CompilerCmdHandlerFactory());
}

CompilerCmdStatementStrategy::~CompilerCmdStatementStrategy() = default;

bool CompilerCmdStatementStrategy::execute(CompilerContext* context) {
  cmd_cmd(context);
  return context->compiled;
}
