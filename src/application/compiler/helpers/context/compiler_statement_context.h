#ifndef COMPILER_STATEMENT_CONTEXT_H_INCLUDED
#define COMPILER_STATEMENT_CONTEXT_H_INCLUDED

#include <functional>

#include "compiler_command_id.h"

class ActionNode;
class Lexeme;

class CompilerStatementContext {
 public:
  ActionNode* action;
  Lexeme* lexeme;
  bool traps_checked;
  bool skip_post_trap_check;
  std::function<bool(CompilerCommandId, bool&)> dispatch;

  CompilerStatementContext()
      : action(0),
        lexeme(0),
        traps_checked(false),
        skip_post_trap_check(false) {}
};

#endif  // COMPILER_STATEMENT_CONTEXT_H_INCLUDED
