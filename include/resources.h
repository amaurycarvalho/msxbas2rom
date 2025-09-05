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
 * @brief Resource reader for SCn files (BLOAD binary screens)
 * @note https://msx.jannone.org/conv/
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
 * @note https://msx.jannone.org/tinysprite/tinysprite.html
 */
class ResourceSprReader : public ResourceReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceSprReader(string filename);

  /// @todo transfer it to load() method
  static int ParseTinySpriteFile(string filename, unsigned char *data,
                                 int maxlen);
};

/***
 * @class ResourceAkmReader
 * @brief Resource reader for Arkos Tracker minimalist player music files (.AKM)
 * @note https://julien-nevo.com/at3test/index.php/download/
 */
class ResourceAkmReader : public ResourceBlobReader {
 private:
  void fix();

 public:
  static bool isValid(string fileext);
  bool load();
  ResourceAkmReader(string filename);

  /// @todo transfer it to fix() method
  static void fixAKM(unsigned char *data, int address, int length);
};

/***
 * @class ResourceAkxReader
 * @brief Resource reader for Arkos Tracker sound effects files (.AKX)
 * @note https://julien-nevo.com/at3test/index.php/download/
 */
class ResourceAkxReader : public ResourceBlobReader {
 private:
  void fix();

 public:
  static bool isValid(string fileext);
  bool load();
  ResourceAkxReader(string filename);

  /// @todo transfer it to fix() method
  static void fixAKX(unsigned char *data, int address, int length);
};

/***
 * @class ResourceMtfReader
 * @brief Resource reader for MSX Tile Forge projects (.mtf.json)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 */
class ResourceMtfReader : public ResourceBlobReader {
 public:
  static bool isValid(string fileext);
  bool load();
  ResourceMtfReader(string filename);
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
};

#endif  // RESOURCES_H_INCLUDED