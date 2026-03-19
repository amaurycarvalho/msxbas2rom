/***
 * @file symbol_manager.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_manager.h"

#include "build_options.h"
#include "symbol_export_context.h"

bool SymbolManager::saveSymbol(shared_ptr<BuildOptions> opts) {
  SymbolExportStrategy* strategy =
      symbolExportFactory.getBySymbolMode(opts->symbols);
  return (strategy) ? strategy->save(context, opts) : false;
}

SymbolManager::SymbolManager() {
  context = make_shared<SymbolExportContext>();
}

SymbolManager::~SymbolManager() = default;
