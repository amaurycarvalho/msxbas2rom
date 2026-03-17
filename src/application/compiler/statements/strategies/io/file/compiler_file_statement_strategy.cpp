#include "compiler_file_statement_strategy.h"

#include "build_options.h"
#include "compiler_context.h"
#include "fswrapper.h"
#include "resources.h"

void CompilerFileStatementStrategy::cmd_file(CompilerContext* context) {
  auto& opts = *context->opts;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
  unsigned int t = context->current_action->actions.size();
  string filename;

  if (t == 1) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "FILE";
      filename = removeQuotes(lexeme->value);
      context->resourceManager->addFile(filename, opts.inputPath);
    } else {
      context->syntaxError("Invalid parameter in FILE keyword");
    }

  } else {
    context->syntaxError("Wrong FILE parameters count");
  }
}

bool CompilerFileStatementStrategy::execute(CompilerContext* context) {
  cmd_file(context);
  return context->compiled;
}
