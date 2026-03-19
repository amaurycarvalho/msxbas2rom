#ifndef COMPILER_STATEMENT_CONTEXT_H_INCLUDED
#define COMPILER_STATEMENT_CONTEXT_H_INCLUDED

#include <functional>
#include <memory>
#include <stack>
#include <string>

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

class TagNode;
class ActionNode;
class SymbolManager;
class ResourceManager;
class Parser;
class BuildOptions;
class Logger;

using namespace std;

class CompilerContext {
 public:
  shared_ptr<Parser> parser;
  shared_ptr<BuildOptions> opts;
  shared_ptr<ICpuOpcodeWriter> cpu;

  shared_ptr<CompilerEvaluator> evaluator;
  shared_ptr<CompilerCodeHelper> codeHelper;
  shared_ptr<CompilerFixupResolver> fixupResolver;
  shared_ptr<CompilerSymbolResolver> symbolResolver;
  shared_ptr<CompilerCodeOptimizer> codeOptimizer;
  shared_ptr<CompilerExpressionEvaluator> expressionEvaluator;
  shared_ptr<CompilerFloatConverter> floatConverter;
  shared_ptr<CompilerVariableEmitter> variableEmitter;

  shared_ptr<SymbolManager> symbolManager;
  shared_ptr<ResourceManager> resourceManager;

  bool pt3, akm, font, file_support, has_defusr;
  bool has_open_grp;
  bool has_tiny_sprite;
  bool traps_checked;
  bool skip_post_trap_check;
  bool compiled;

  int mark_count, for_count;

  shared_ptr<TagNode> current_tag;
  shared_ptr<SymbolNode> heap_mark;
  shared_ptr<SymbolNode> temp_str_mark;
  shared_ptr<SymbolNode> end_mark;
  shared_ptr<FixNode> enable_basic_mark, disable_basic_mark;
  shared_ptr<FixNode> draw_mark;
  shared_ptr<FixNode> io_redirect_mark, io_screen_mark;
  shared_ptr<ActionNode> current_action;

  vector<shared_ptr<SymbolNode>> symbols;
  vector<shared_ptr<FixNode>> fixes;
  stack<shared_ptr<ForNextNode>> forNextStack;

  shared_ptr<Logger> logger;

  void setHelpers(shared_ptr<CompilerContext> context);

  void clear();

  bool containErrors();

  void syntaxError();

  void syntaxError(string msg);

  CompilerContext();
  ~CompilerContext();
};

#endif  // COMPILER_STATEMENT_CONTEXT_H_INCLUDED
