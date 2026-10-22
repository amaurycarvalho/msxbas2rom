/***
 * @file symbol_manager.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_manager.h"

#include "build_options.h"
#include "symbol_export_context.h"
#include "symbol_output_sink.h"

bool SymbolManager::saveSymbol(shared_ptr<BuildOptions> opts) {
  SymbolExportStrategy* strategy =
      symbolExportFactory.getBySymbolMode(opts->symbols);
  return (strategy) ? strategy->save(context, opts, sink) : false;
}

SymbolManager::SymbolManager(shared_ptr<SymbolOutputSink> outputSink) {
  context = make_shared<SymbolExportContext>();
  sink = outputSink;
}

SymbolManager::~SymbolManager() = default;
