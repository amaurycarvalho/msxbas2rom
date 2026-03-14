/***
 * @file symbol_export_strategy_factory.cpp
 * @brief Factory for symbol export strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "symbol_export_strategy_factory.h"

SymbolExportStrategyFactory::SymbolExportStrategyFactory() {
  strategies[BuildOptions::SymbolsMode::Symbol] = &symbolExport;
  strategies[BuildOptions::SymbolsMode::Omds] = &omdsExport;
  strategies[BuildOptions::SymbolsMode::Cdb] = &cdbExport;
  strategies[BuildOptions::SymbolsMode::NoICE] = &noiceExport;
  strategies[BuildOptions::SymbolsMode::Elf] = &elfExport;
}

SymbolExportStrategyFactory::~SymbolExportStrategyFactory() = default;

SymbolExportStrategy* SymbolExportStrategyFactory::getBySymbolMode(
    BuildOptions::SymbolsMode mode) {
  auto it = strategies.find(mode);

  if (it == strategies.end()) return nullptr;

  return it->second;
}

size_t SymbolExportStrategyFactory::size() const {
  return strategies.size();
}
