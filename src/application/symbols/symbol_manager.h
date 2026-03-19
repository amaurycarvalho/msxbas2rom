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

#include "symbol_export_strategy_factory.h"

class BuildOptions;
class SymbolExportContext;

using namespace std;

/***
 * @class SymbolManager
 * @brief Symbol manager
 */
class SymbolManager {
 private:
  SymbolExportStrategyFactory symbolExportFactory;

 public:
  shared_ptr<SymbolExportContext> context;

  bool saveSymbol(shared_ptr<BuildOptions> opts);

  SymbolManager();
  ~SymbolManager();
};

#endif  // SYMBOL_MANAGER_H_INCLUDED
