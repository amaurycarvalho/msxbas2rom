#ifndef COMPILER_STATEMENT_CONTEXT_H_INCLUDED
#define COMPILER_STATEMENT_CONTEXT_H_INCLUDED

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>

#include "compiler_code_optimizer.h"
#include "dispatch_fixup.h"
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
class KernelImage;
class ResourceContentSource;
class SymbolOutputSink;

using namespace std;

class CompilerContext {
 private:
  shared_ptr<Parser> parser;
  shared_ptr<BuildOptions> opts;
  shared_ptr<ICpuOpcodeWriter> cpu;
  shared_ptr<KernelImage> kernelImage;

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
  shared_ptr<FixNode> enableRomBasicMark, disableRomBasicMark;
  shared_ptr<FixNode> drawStmtMark;
  shared_ptr<FixNode> ioRedirectMark, ioScreenMark;
  shared_ptr<ActionNode> current_action;

  vector<shared_ptr<SymbolNode>> symbols;
  vector<shared_ptr<FixNode>> fixes;
  set<unsigned int> usedDispatch;
  map<unsigned int, unsigned int> dispatchAddresses;
  vector<DispatchFixup> dispatchFixups;
  stack<shared_ptr<ForNextNode>> forNextStack;

  shared_ptr<Logger> logger;


 public:
  shared_ptr<Parser>& getParser() { return parser; }
  shared_ptr<BuildOptions>& getOpts() { return opts; }
  shared_ptr<ICpuOpcodeWriter>& getCpu() { return cpu; }
  shared_ptr<KernelImage>& getKernelImage() { return kernelImage; }

  shared_ptr<CompilerEvaluator>& getEvaluator() { return evaluator; }
  shared_ptr<CompilerCodeHelper>& getCodeHelper() { return codeHelper; }
  shared_ptr<CompilerFixupResolver>& getFixupResolver() { return fixupResolver; }
  shared_ptr<CompilerSymbolResolver>& getSymbolResolver() { return symbolResolver; }
  shared_ptr<CompilerCodeOptimizer>& getCodeOptimizer() { return codeOptimizer; }
  shared_ptr<CompilerExpressionEvaluator>& getExpressionEvaluator() {
    return expressionEvaluator;
  }
  shared_ptr<CompilerFloatConverter>& getFloatConverter() { return floatConverter; }
  shared_ptr<CompilerVariableEmitter>& getVariableEmitter() {
    return variableEmitter;
  }

  shared_ptr<SymbolManager>& getSymbolManager() { return symbolManager; }
  shared_ptr<ResourceManager>& getResourceManager() { return resourceManager; }

  bool& getPt3() { return pt3; }
  bool& getAkm() { return akm; }
  bool& getFont() { return font; }
  bool& getFileSupport() { return file_support; }
  bool& getHasDefusr() { return has_defusr; }
  bool& getHasOpenGrp() { return has_open_grp; }
  bool& getHasTinySprite() { return has_tiny_sprite; }
  bool& getTrapsChecked() { return traps_checked; }
  bool& getSkipPostTrapCheck() { return skip_post_trap_check; }
  bool& getCompiled() { return compiled; }

  int& getMarkCount() { return mark_count; }
  int& getForCount() { return for_count; }

  shared_ptr<TagNode>& getCurrentTag() { return current_tag; }
  shared_ptr<SymbolNode>& getHeapMark() { return heap_mark; }
  shared_ptr<SymbolNode>& getTempStrMark() { return temp_str_mark; }
  shared_ptr<SymbolNode>& getEndMark() { return end_mark; }
  shared_ptr<FixNode>& getEnableRomBasicMark() { return enableRomBasicMark; }
  shared_ptr<FixNode>& getDisableRomBasicMark() { return disableRomBasicMark; }
  shared_ptr<FixNode>& getDrawStmtMark() { return drawStmtMark; }
  shared_ptr<FixNode>& getIoRedirectMark() { return ioRedirectMark; }
  shared_ptr<FixNode>& getIoScreenMark() { return ioScreenMark; }
  shared_ptr<ActionNode>& getCurrentAction() { return current_action; }

  vector<shared_ptr<SymbolNode>>& getSymbols() { return symbols; }
  vector<shared_ptr<FixNode>>& getFixes() { return fixes; }
  set<unsigned int>& getUsedDispatch() { return usedDispatch; }
  map<unsigned int, unsigned int>& getDispatchAddresses() {
    return dispatchAddresses;
  }
  vector<DispatchFixup>& getDispatchFixups() { return dispatchFixups; }
  stack<shared_ptr<ForNextNode>>& getForNextStack() { return forNextStack; }

  shared_ptr<Logger>& getLogger() { return logger; }

  void setHelpers(shared_ptr<CompilerContext> context);

  void clear();

  bool containErrors();

  void syntaxError();

  void syntaxError(string msg);

  CompilerContext(shared_ptr<KernelImage> kernelImage,
                  shared_ptr<ResourceContentSource> resourceContentSource,
                  shared_ptr<SymbolOutputSink> symbolOutputSink);
  ~CompilerContext();
};

#endif  // COMPILER_STATEMENT_CONTEXT_H_INCLUDED
