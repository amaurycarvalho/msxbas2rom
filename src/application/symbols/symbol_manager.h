/***
 * @file symbol_manager.h
 * @brief SymbolManager class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_MANAGER_H_INCLUDED
#define SYMBOL_MANAGER_H_INCLUDED

#include <string.h>

#include <queue>
#include <vector>

#include "code_node.h"
#include "options.h"

using namespace std;

/***
 * @class SymbolManager
 * @brief Symbol manager
 */
class SymbolManager {
 private:
  bool saveWithStrategy(class SymbolExportStrategy* strategy,
                        BuildOptions* opts);

 public:
  vector<vector<string>> getKernelSymbolAddresses();
  vector<CodeNode*> codeList;
  vector<CodeNode*> dataList;

  /***
   * @brief clear all symbol lists
   */
  void clear();

  bool saveSymbol(BuildOptions* opts);
};

#endif  // SYMBOL_MANAGER_H_INCLUDED
