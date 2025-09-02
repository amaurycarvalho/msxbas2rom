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
   * @param code_start code start position on RAM memory
   * @param ram_page data start position on RAM memory
   * @note https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp
   */
  bool saveSymbolFile(BuildOptions *opts, int code_start, int ram_page);
};

#endif  // RESOURCES_H_INCLUDED