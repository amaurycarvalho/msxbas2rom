#ifndef COMPILER_STATEMENT_CONTEXT_H_INCLUDED
#define COMPILER_STATEMENT_CONTEXT_H_INCLUDED

#include <functional>

#include "cpu_opcode_writer.h"
#include "fix_node.h"
#include "for_next_node.h"
#include "parser.h"
#include "resources.h"
#include "symbol_manager.h"
#include "symbol_node.h"

class CompilerSymbolResolver;
class CompilerFixupResolver;
class CompilerCodeHelper;
class CompilerCodeOptimizer;
class CompilerExpressionEvaluator;
class CompilerFloatConverter;
class CompilerVariableEmitter;
class CompilerStatementEmitter;
class CompilerEvaluator;

using namespace std;

class CompilerContext {
 public:
  Parser* parser;
  BuildOptions* opts;
  ICpuOpcodeWriter* cpu;

  CompilerEvaluator* evaluator;
  CompilerStatementEmitter* stmtEmitter;
  CompilerCodeHelper* codeHelper;
  CompilerFixupResolver* fixupResolver;
  CompilerSymbolResolver* symbolResolver;
  CompilerCodeOptimizer* codeOptimizer;
  CompilerExpressionEvaluator* expressionEvaluator;
  CompilerFloatConverter* floatConverter;
  CompilerVariableEmitter* variableEmitter;

  SymbolManager symbolManager;
  ResourceManager resourceManager;

  bool pt3, akm, font, file_support, has_defusr;
  bool has_open_grp;
  bool has_tiny_sprite;
  bool traps_checked;
  bool skip_post_trap_check;
  bool compiled;

  int mark_count, for_count;

  TagNode* current_tag;
  SymbolNode *heap_mark, *temp_str_mark;
  SymbolNode* end_mark;
  FixNode *enable_basic_mark, *disable_basic_mark;
  FixNode* draw_mark;
  FixNode *io_redirect_mark, *io_screen_mark;
  ActionNode* current_action;

  vector<SymbolNode*> symbols;
  vector<FixNode*> fixes;
  stack<ForNextNode*> forNextStack;

  string error_message;

  void clear();
  void syntaxError();
  void syntaxError(string msg);

  CompilerContext();
  ~CompilerContext();
};

#endif  // COMPILER_STATEMENT_CONTEXT_H_INCLUDED
