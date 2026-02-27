/***
 * @file symbol_export_strategy_factory.cpp
 * @brief Factory for symbol export strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "symbol_export_strategy_factory.h"

#include "cdb_export_strategy.h"
#include "noice_export_strategy.h"
#include "omds_export_strategy.h"
#include "symbol_file_export_strategy.h"

unique_ptr<SymbolExportStrategy> SymbolExportStrategyFactory::create(
    BuildOptions::SymbolsMode mode) {
  switch (mode) {
    case BuildOptions::SymbolsMode::Symbol:
      return unique_ptr<SymbolExportStrategy>(new SymbolFileExportStrategy());
    case BuildOptions::SymbolsMode::Omds:
      return unique_ptr<SymbolExportStrategy>(new OmdsExportStrategy());
    case BuildOptions::SymbolsMode::Cdb:
      return unique_ptr<SymbolExportStrategy>(new CdbExportStrategy());
    case BuildOptions::SymbolsMode::NoICE:
      return unique_ptr<SymbolExportStrategy>(new NoIceExportStrategy());
    default:
      return NULL;
  }
}
