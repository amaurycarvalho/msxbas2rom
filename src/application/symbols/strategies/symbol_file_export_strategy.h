/***
 * @file symbol_file_export_strategy.h
 * @brief Symbol (*.symbol) export strategy
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_FILE_EXPORT_STRATEGY_H_INCLUDED
#define SYMBOL_FILE_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

/***
 * @brief Write symbols file (*.symbol) to use with old/new OpenMSX Debugger
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
class SymbolFileExportStrategy : public SymbolExportStrategy {
 public:
  bool save(SymbolManager* symbolManager, BuildOptions* opts);
};

#endif  // SYMBOL_FILE_EXPORT_STRATEGY_H_INCLUDED
