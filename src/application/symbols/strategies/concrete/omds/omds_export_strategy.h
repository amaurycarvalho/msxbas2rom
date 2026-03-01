/***
 * @file omds_export_strategy.h
 * @brief OMDS (*.omds) export strategy
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef OMDS_EXPORT_STRATEGY_H_INCLUDED
#define OMDS_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

/***
 * @brief Write OMDS file (*.omds) to use with old OpenMSX Debugger
 * @param opts compiler options
 * @note
 * [old
 * debugger](https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp)
 * ; [new
 * debugger](https://github.com/openMSX/openMSX/blob/master/src/debugger/SymbolManager.cc)
 * ; [new debugger unit
 * test](https://github.com/openMSX/openMSX/blob/master/src/unittest/SymbolManager_test.cc)
 * @remark depends on compile->write and compile->save_symbols methods to
 * show the correct symbol addresses
 */

class OmdsExportStrategy : public SymbolExportStrategy {
 public:
  bool save(SymbolManager* symbolManager, BuildOptions* opts);
};

#endif  // OMDS_EXPORT_STRATEGY_H_INCLUDED
