/***
 * @file resource_txt_reader.h
 * @brief Resource text reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_TXT_READER_H_INCLUDED
#define RESOURCE_TXT_READER_H_INCLUDED

#include "resource_reader.h"

/***
 * @class ResourceTxtReader
 * @brief Resource reader for plain text files
 * @remark
 *   TXT resource structure:
 *     resourceType N(1) = 2
 *     lineList:
 *       lineSize N(1)
 *       lineData C(lineSize)
 */
class ResourceTxtReader : public ResourceReader {
 public:
  vector<string> lines;
  static bool isIt(string fileext);
  bool populateLines();
  bool load();
  ResourceTxtReader(string filename);
};

#endif  // RESOURCE_TXT_READER_H_INCLUDED
