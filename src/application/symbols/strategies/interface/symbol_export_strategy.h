/***
 * @file symbol_export_strategy.h
 * @brief Symbol export strategy interface
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef SYMBOL_EXPORT_STRATEGY_H_INCLUDED
#define SYMBOL_EXPORT_STRATEGY_H_INCLUDED

#include <memory>

class SymbolExportContext;
class BuildOptions;

using namespace std;

class SymbolExportStrategy {
 public:
  virtual ~SymbolExportStrategy() {}

  virtual bool save(shared_ptr<SymbolExportContext> context,
                    shared_ptr<BuildOptions> opts) = 0;
};

#endif  // SYMBOL_EXPORT_STRATEGY_H_INCLUDED
