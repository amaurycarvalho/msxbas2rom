/***
 * @file symbol_export_strategy.h
 * @brief Symbol export strategy interface
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_EXPORT_CONTEXT_H_INCLUDED
#define SYMBOL_EXPORT_CONTEXT_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class CodeNode;

using namespace std;

class SymbolExportContext {
 public:
  string exportFilename;
  vector<vector<string>> kernelSymbolAddresses;

  vector<shared_ptr<CodeNode>> codeList;
  vector<shared_ptr<CodeNode>> dataList;

  /***
   * @brief clear all symbol lists
   */
  void clear();

  SymbolExportContext();
  ~SymbolExportContext();
};

#endif  // SYMBOL_EXPORT_CONTEXT_H_INCLUDED
