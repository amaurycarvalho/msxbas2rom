/***
 * @file cdb_export_strategy.h
 * @brief CDB (*.cdb) dummy export strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef CDB_EXPORT_STRATEGY_H_INCLUDED
#define CDB_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

/***
 * @brief Write CDB file (*.cdb) as a dummy output placeholder
 * @param opts compiler options
 */
class CdbExportStrategy : public SymbolExportStrategy {
 public:
  bool save(SymbolManager* symbolManager, BuildOptions* opts);
};

#endif  // CDB_EXPORT_STRATEGY_H_INCLUDED
