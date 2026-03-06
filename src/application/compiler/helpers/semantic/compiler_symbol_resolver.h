/***
 * @file compiler_symbol_resolver.h
 * @brief Compiler symbol resolver
 */

#ifndef COMPILER_SYMBOL_RESOLVER_H
#define COMPILER_SYMBOL_RESOLVER_H

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
  SymbolNode* getSymbol(Lexeme* lexeme);
  SymbolNode* addSymbol(Lexeme* lexeme);
  SymbolNode* getSymbol(TagNode* tag);
  SymbolNode* addSymbol(TagNode* tag);
  SymbolNode* addSymbol(string line);

  void addSupportSymbols();
  void clearSymbols();
  int saveSymbols();

  CompilerSymbolResolver(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_SYMBOL_RESOLVER_H