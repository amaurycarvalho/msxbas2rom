/***
 * @file compiler_fixup_resolver.h
 * @brief Compiler fixup resolver
 */

#ifndef COMPILER_FIXUP_RESOLVER_H
#define COMPILER_FIXUP_RESOLVER_H

#include <memory>
#include <string>

class CompilerContext;
class SymbolNode;
class FixNode;
class Lexeme;
class TagNode;

using namespace std;

class CompilerFixupResolver {
 private:
  shared_ptr<CompilerContext> context;

 public:
  shared_ptr<FixNode> addFix(shared_ptr<Lexeme> lexeme);
  shared_ptr<FixNode> addFix(shared_ptr<SymbolNode> symbol);
  shared_ptr<FixNode> addFix(string line);
  shared_ptr<SymbolNode> addPreMark();
  shared_ptr<FixNode> addMark();
  void doFix();

  CompilerFixupResolver(shared_ptr<CompilerContext> context);
  ~CompilerFixupResolver();
};

#endif  // COMPILER_FIXUP_RESOLVER_H