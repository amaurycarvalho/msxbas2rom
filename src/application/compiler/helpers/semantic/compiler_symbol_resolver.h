/***
 * @file compiler_symbol_resolver.h
 * @brief Compiler symbol resolver
 */

#ifndef COMPILER_SYMBOL_RESOLVER_H
#define COMPILER_SYMBOL_RESOLVER_H

#include <memory>
#include <string>

class CompilerContext;
class SymbolNode;
class Lexeme;
class TagNode;

using namespace std;

class CompilerSymbolResolver {
 private:
  shared_ptr<CompilerContext> context;

 public:
  shared_ptr<SymbolNode> getSymbol(shared_ptr<Lexeme> lexeme);
  shared_ptr<SymbolNode> addSymbol(shared_ptr<Lexeme> lexeme);
  shared_ptr<SymbolNode> getSymbol(shared_ptr<TagNode> tag);
  shared_ptr<SymbolNode> addSymbol(shared_ptr<TagNode> tag);
  shared_ptr<SymbolNode> addSymbol(string line);

  void addSupportSymbols();
  void clearSymbols();
  int saveSymbols();

  CompilerSymbolResolver(shared_ptr<CompilerContext> context);
  ~CompilerSymbolResolver();
};

#endif  // COMPILER_SYMBOL_RESOLVER_H