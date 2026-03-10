#include "compiler_cmd_pt3_handler.h"

#include "compiler_context.h"

bool CompilerCmdPt3Handler::execute(CompilerContext* context,
                                    ActionNode* action) {
  context->pt3 = true;
  context->syntaxError("Arkos Tracker not permitted with PT3 player");

  return context->compiled;
}
