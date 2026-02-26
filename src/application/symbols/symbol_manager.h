/***
 * @file symbol_manager.h
 * @brief SymbolManager class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_MANAGER_H_INCLUDED
#define SYMBOL_MANAGER_H_INCLUDED

#include <string.h>

#include <queue>
#include <vector>

#include "code_node.h"
#include "options.h"

/***
 * @class SymbolManager
 * @brief Symbol manager
 */
class SymbolManager {
 private:
  std::vector<std::vector<std::string>> getKernelSymbolAddresses();
  bool saveWithStrategy(class SymbolExportStrategy* strategy,
                        BuildOptions* opts);

 public:
  std::vector<CodeNode*> codeList;
  std::vector<CodeNode*> dataList;

  /***
   * @brief clear all symbol lists
   */
  void clear();

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
  bool saveSymbolFile(BuildOptions* opts);

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
  bool saveNoIceFile(BuildOptions* opts);

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
  bool saveOmdsFile(BuildOptions* opts);
};

#endif  // SYMBOL_MANAGER_H_INCLUDED
