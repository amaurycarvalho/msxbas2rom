/***
 * @file symbol_export_strategy.h
 * @brief Symbol export strategy interface
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_EXPORT_STRATEGY_H_INCLUDED
#define SYMBOL_EXPORT_STRATEGY_H_INCLUDED

#include <memory>
#include <string>

class SymbolManager;
class BuildOptions;

using namespace std;

class SymbolExportStrategy {
 public:
  virtual ~SymbolExportStrategy() {}

  virtual bool save(SymbolManager* symbolManager,
                    shared_ptr<BuildOptions> opts) = 0;
};

#endif  // SYMBOL_EXPORT_STRATEGY_H_INCLUDED
