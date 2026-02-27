/***
 * @file symbol_export_strategy_factory.h
 * @brief Factory for symbol export strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef SYMBOL_EXPORT_STRATEGY_FACTORY_H_INCLUDED
#define SYMBOL_EXPORT_STRATEGY_FACTORY_H_INCLUDED

#include <memory>

#include "options.h"
#include "symbol_export_strategy.h"

class SymbolExportStrategyFactory {
 public:
  static unique_ptr<SymbolExportStrategy> create(
      BuildOptions::SymbolsMode mode);
};

#endif  // SYMBOL_EXPORT_STRATEGY_FACTORY_H_INCLUDED
