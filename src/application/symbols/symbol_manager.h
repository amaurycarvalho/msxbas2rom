/***
 * @file symbol_manager.h
 * @brief SymbolManager class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_MANAGER_H_INCLUDED
#define SYMBOL_MANAGER_H_INCLUDED

#include <memory>
#include <queue>
#include <string>
#include <vector>

class CodeNode;
class BuildOptions;
class SymbolExportStrategyFactory;

using namespace std;

/***
 * @class SymbolManager
 * @brief Symbol manager
 */
class SymbolManager {
 private:
  unique_ptr<SymbolExportStrategyFactory> symbolExportFactory;

 public:
  vector<vector<string>> getKernelSymbolAddresses();
  vector<CodeNode*> codeList;
  vector<CodeNode*> dataList;

  /***
   * @brief clear all symbol lists
   */
  void clear();

  bool saveSymbol(BuildOptions* opts);
  SymbolManager();
  ~SymbolManager();
};

#endif  // SYMBOL_MANAGER_H_INCLUDED
