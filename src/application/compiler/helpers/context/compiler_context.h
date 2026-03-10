#ifndef COMPILER_STATEMENT_CONTEXT_H_INCLUDED
#define COMPILER_STATEMENT_CONTEXT_H_INCLUDED

#include <functional>
#include <memory>
#include <stack>

#include "cpu_opcode_writer.h"
#include "for_next_node.h"
#include "symbol_node.h"

class CompilerSymbolResolver;
class CompilerFixupResolver;
class CompilerCodeHelper;
class CompilerCodeOptimizer;
class CompilerExpressionEvaluator;
class CompilerFloatConverter;
class CompilerVariableEmitter;
class CompilerEvaluator;

class SymbolManager;
class ResourceManager;
class Parser;
class BuildOptions;

using namespace std;

class CompilerContext {
 public:
  Parser* parser;
  BuildOptions* opts;
  ICpuOpcodeWriter* cpu;

  unique_ptr<CompilerEvaluator> evaluator;
  unique_ptr<CompilerCodeHelper> codeHelper;
  unique_ptr<CompilerFixupResolver> fixupResolver;
  unique_ptr<CompilerSymbolResolver> symbolResolver;
  unique_ptr<CompilerCodeOptimizer> codeOptimizer;
  unique_ptr<CompilerExpressionEvaluator> expressionEvaluator;
  unique_ptr<CompilerFloatConverter> floatConverter;
  unique_ptr<CompilerVariableEmitter> variableEmitter;

  unique_ptr<SymbolManager> symbolManager;
  unique_ptr<ResourceManager> resourceManager;

  bool pt3, akm, font, file_support, has_defusr;
  bool has_open_grp;
  bool has_tiny_sprite;
  bool traps_checked;
  bool skip_post_trap_check;
  bool compiled;

  int mark_count, for_count;

  TagNode* current_tag;
  unique_ptr<SymbolNode> heap_mark;
  unique_ptr<SymbolNode> temp_str_mark;
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
