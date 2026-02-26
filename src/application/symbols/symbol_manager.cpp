/***
 * @file symbol_manager.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_manager.h"

#include "noice_export_strategy.h"
#include "omds_export_strategy.h"
#include "symbol_file_export_strategy.h"

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

bool SymbolManager::saveSymbolFile(BuildOptions* opts) {
  SymbolFileExportStrategy strategy;
  return saveWithStrategy(&strategy, opts);
}

bool SymbolManager::saveNoIceFile(BuildOptions* opts) {
  NoIceExportStrategy strategy;
  return saveWithStrategy(&strategy, opts);
}

bool SymbolManager::saveOmdsFile(BuildOptions* opts) {
  OmdsExportStrategy strategy;
  return saveWithStrategy(&strategy, opts);
}

bool SymbolManager::saveWithStrategy(SymbolExportStrategy* strategy,
                                     BuildOptions* opts) {
  return strategy->save(getKernelSymbolAddresses(), codeList, dataList, opts);
}
