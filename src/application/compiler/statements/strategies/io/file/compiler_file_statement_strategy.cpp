#include "compiler_file_statement_strategy.h"

#include "action_node.h"
#include "build_options.h"
#include "compiler_context.h"
#include <strings.h>

#include "build_path_policy.h"
#include "lexeme.h"
#include "resources.h"

void CompilerFileStatementStrategy::cmd_file(
    shared_ptr<CompilerContext> context) {
  auto& opts = *context->getOpts();
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
  unsigned int t = context->getCurrentAction()->actions.size();
  string filename;

  if (t == 1) {
    action = context->getCurrentAction()->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "FILE";
      filename = build_path::removeQuotes(lexeme->value);
      context->getResourceManager()->addFile(filename, opts.inputPath);
    } else {
      context->syntaxError("Invalid parameter in FILE keyword");
    }

  } else {
    context->syntaxError("Wrong FILE parameters count");
  }
}

bool CompilerFileStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_file(context);
  return context->getCompiled();
}
