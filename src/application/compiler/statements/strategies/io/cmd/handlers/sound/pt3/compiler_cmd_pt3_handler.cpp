#include "compiler_cmd_pt3_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "lexeme.h"

bool CompilerCmdPt3Handler::execute(shared_ptr<CompilerContext> context,
                                    shared_ptr<ActionNode> action) {
  context->pt3 = true;
  context->syntaxError("Arkos Tracker not permitted with PT3 player");

  return context->compiled;
}
