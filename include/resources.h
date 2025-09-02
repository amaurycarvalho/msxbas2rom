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
  vector<Lexeme*> resourceList;
  vector<FileNode*> fileList;
  vector<CodeNode*> codeList;
  vector<CodeNode*> dataList;

  void clear();
};

#endif  // RESOURCES_H_INCLUDED