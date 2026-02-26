/***
 * @file symbol_manager.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_manager.h"

#include "symbol_export_strategy_factory.h"

void SymbolManager::clear() {
  codeList.clear();
  dataList.clear();
}

std::vector<std::vector<std::string>>
SymbolManager::getKernelSymbolAddresses() {
  return {
      {"LOADER", "4010", "jump"},        {"VAR_CURSEGM", "C023", "variable"},
      {"MR_CALL", "41C8", "jump"},       {"MR_CALL_TRAP", "41CB", "jump"},
      {"MR_CHANGE_SGM", "41CE", "jump"}, {"MR_GET_BYTE", "41D1", "jump"},
      {"MR_GET_DATA", "41D4", "jump"},   {"MR_JUMP", "41D7", "jump"}};
}

bool SymbolManager::saveSymbol(BuildOptions* opts) {
  std::unique_ptr<SymbolExportStrategy> strategy =
      SymbolExportStrategyFactory::create(opts->symbols);
  return saveWithStrategy(strategy.get(), opts);
}

bool SymbolManager::saveWithStrategy(SymbolExportStrategy* strategy,
                                     BuildOptions* opts) {
  return (strategy) ? strategy->save(this, opts) : false;
}
