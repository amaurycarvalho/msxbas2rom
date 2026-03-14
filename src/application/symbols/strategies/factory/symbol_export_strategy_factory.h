/***
 * @file symbol_export_strategy_factory.h
 * @brief Factory for symbol export strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef SYMBOL_EXPORT_STRATEGY_FACTORY_H_INCLUDED
#define SYMBOL_EXPORT_STRATEGY_FACTORY_H_INCLUDED

#include <map>

#include "build_options.h"
#include "cdb_export_strategy.h"
#include "elf_export_strategy.h"
#include "noice_export_strategy.h"
#include "omds_export_strategy.h"
#include "symbol_export_strategy.h"
#include "symbol_file_export_strategy.h"

class SymbolExportStrategyFactory {
 private:
  SymbolFileExportStrategy symbolExport;
  OmdsExportStrategy omdsExport;
  CdbExportStrategy cdbExport;
  NoIceExportStrategy noiceExport;
  ElfExportStrategy elfExport;

  map<BuildOptions::SymbolsMode, SymbolExportStrategy*> strategies;

 public:
  SymbolExportStrategyFactory();
  ~SymbolExportStrategyFactory();
  SymbolExportStrategy* getBySymbolMode(BuildOptions::SymbolsMode mode);
  size_t size() const;
};

#endif  // SYMBOL_EXPORT_STRATEGY_FACTORY_H_INCLUDED
