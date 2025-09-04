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
 * @class ResourceReader
 * @brief Resource reader factory
 */
class ResourceReader {
 public:
  string filename;
  vector<vector<unsigned char>> data;
  static ResourceReader *create(string filename);
  static bool isValid(string fileext);
  bool load();
  ResourceReader(string filename);
};

/***
 * @class ResourceBlobReader
 * @brief Resource reader for uncompressed binary files (limited to 16k)
 */
class ResourceBlobReader : public ResourceReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceBlobReader(string filename);
};

/***
 * @class ResourceTxtReader
 * @brief Resource reader for plain text files
 */
class ResourceTxtReader : public ResourceReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceTxtReader(string filename);
};

/***
 * @class ResourceCsvReader
 * @brief Resource reader for CSV files
 */
class ResourceCsvReader : public ResourceReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceCsvReader(string filename);
};

/***
 * @class ResourceScrReader
 * @brief Resource reader for SCn files (binary screens)
 */
class ResourceScrReader : public ResourceReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceScrReader(string filename);
};

/***
 * @class ResourceSprReader
 * @brief Resource reader for SPR files (TinySprite plain text files)
 */
class ResourceSprReader : public ResourceReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceSprReader(string filename);
};

/***
 * @class ResourceDataReader
 * @brief Resource reader for DATA statements
 */
class ResourceDataReader : public ResourceReader {
 private:
  Parser *parser;

 public:
  bool load();
  ResourceDataReader(Parser *parser);
};

/***
 * @class ResourceManager
 * @brief Resource manager
 */
class ResourceManager {
 private:
  vector<ResourceReader *> resourceReaderList;

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
   * @brief add a new resource
   */
  bool add(string filename);

  /***
   * @brief Add DATA statement resource to the resource list
   */
  void addDataResource(Parser *parser);

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
};

#endif  // RESOURCES_H_INCLUDED