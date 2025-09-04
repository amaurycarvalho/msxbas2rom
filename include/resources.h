/***
 * @file resources.h
 * @brief Resources class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

#include "symbols.h"

/***
 * @class ResourceManager
 * @brief Resource manager
 */
class ResourceManager {
 public:
  vector<Lexeme *> resourceList;
  vector<FileNode *> fileList;
  vector<CodeNode *> codeList;
  vector<CodeNode *> dataList;

  /***
   * @brief clear all resources
   */
  void clear();

  /***
   * @brief Write symbols file to use with OpenMSX
   * @param opts compiler options
   * @note https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp
   * @remark depends on compile->write and compile->save_symbols methods to
   * show the correct symbol addresses
   */
  bool saveSymbolFile(BuildOptions *opts);

  /***
   * @brief Write OMDS file to use with OpenMSX Debugger
   * @param opts compiler options
   * @note https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp
   * @remark depends on compile->write and compile->save_symbols methods to
   * show the correct symbol addresses
   */
  bool saveOmdsFile(BuildOptions *opts);

  /***
   * @brief Add DATA statement resource to the resource list
   */
  void addDataResource();
};

#endif  // RESOURCES_H_INCLUDED