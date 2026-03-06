/***
 * @file compiler_fixup_resolver.h
 * @brief Compiler fixup resolver
 */

#ifndef COMPILER_FIXUP_RESOLVER_H
#define COMPILER_FIXUP_RESOLVER_H

#include <string>

class CompilerContext;
class SymbolNode;
class FixNode;
class Lexeme;
class TagNode;

using namespace std;

class CompilerFixupResolver {
 private:
  CompilerContext* context;

 public:
  FixNode* addFix(Lexeme* lexeme);
  FixNode* addFix(SymbolNode* symbol);
  FixNode* addFix(string line);
  SymbolNode* addPreMark();
  FixNode* addMark();
  void doFix();

  CompilerFixupResolver(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_FIXUP_RESOLVER_H