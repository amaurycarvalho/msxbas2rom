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
  CompilerContext* context;

 public:
  SymbolNode* getSymbol(shared_ptr<Lexeme> lexeme);
  SymbolNode* addSymbol(shared_ptr<Lexeme> lexeme);
  SymbolNode* getSymbol(TagNode* tag);
  SymbolNode* addSymbol(TagNode* tag);
  SymbolNode* addSymbol(string line);

  void addSupportSymbols();
  void clearSymbols();
  int saveSymbols();

  CompilerSymbolResolver(CompilerContext* context);
  ~CompilerSymbolResolver();
};

#endif  // COMPILER_SYMBOL_RESOLVER_H