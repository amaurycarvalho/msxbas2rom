/***
 * @file symbol_manager.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_manager.h"

#include "build_options.h"
#include "symbol_export_strategy_factory.h"

void SymbolManager::clear() {
  codeList.clear();
  dataList.clear();
}

vector<vector<string>> SymbolManager::getKernelSymbolAddresses() const {
  return kernelSymbolAddresses;
}

bool SymbolManager::saveSymbol(shared_ptr<BuildOptions> opts) {
  SymbolExportStrategy* strategy =
      symbolExportFactory->getBySymbolMode(opts->symbols);
  return (strategy) ? strategy->save(this, opts) : false;
}

SymbolManager::SymbolManager() {
  symbolExportFactory.reset(new SymbolExportStrategyFactory());
  kernelSymbolAddresses = {{"LOADER", "4010", "jump"},
                           {"VAR_CURSEGM", "C023", "variable,byte,1"},
                           {"MR_CALL", "41C8", "jump"},
                           {"MR_CALL_TRAP", "41CB", "jump"},
                           {"MR_CHANGE_SGM", "41CE", "jump"},
                           {"MR_GET_BYTE", "41D1", "jump"},
                           {"MR_GET_DATA", "41D4", "jump"},
                           {"MR_JUMP", "41D7", "jump"}};
}

SymbolManager::~SymbolManager() = default;
