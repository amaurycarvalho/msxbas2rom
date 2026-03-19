#include "compiler_cmd_statement_strategy.h"

#include "action_node.h"
#include "compiler_cmd_handler_factory.h"
#include "compiler_context.h"
#include "lexeme.h"

void CompilerCmdStatementStrategy::cmd_cmd(
    shared_ptr<CompilerContext> context) {
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
  factory = make_shared<CompilerCmdHandlerFactory>();
}

CompilerCmdStatementStrategy::~CompilerCmdStatementStrategy() = default;

bool CompilerCmdStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_cmd(context);
  return context->compiled;
}
