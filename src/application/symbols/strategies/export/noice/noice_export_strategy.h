/***
 * @file noice_export_strategy.h
 * @brief NoICE (*.noi) export strategy
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef NOICE_EXPORT_STRATEGY_H_INCLUDED
#define NOICE_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

/***
 * @brief Write NoICE file (*.noi) to use with new OpenMSX Debugger
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
class NoIceExportStrategy : public SymbolExportStrategy {
 public:
  bool save(SymbolManager* symbolManager, BuildOptions* opts);
};

#endif  // NOICE_EXPORT_STRATEGY_H_INCLUDED
