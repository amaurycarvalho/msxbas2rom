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
 * @brief Resource reader base class (abstract)
 */
class ResourceReader {
 protected:
  string filename;
  FileNode file;

 public:
  vector<vector<unsigned char>> data;
  string getFilename();
  virtual bool load() = 0;
  ResourceReader(string filename);
};

/***
 * @class ResourceFactory
 * @brief Resource factory
 */
class ResourceFactory {
 public:
  static ResourceReader *create(string filename);
};

/***
 * @class ResourceBlobReader
 * @brief Resource reader for binary files (limited to 16k)
 * @note Resource will be saved uncompressed
 * @remark
 *   BLOB resource structure:
 *     blobData C(blobSize)
 * @todo NOT IMPLEMENTED YET
 */
class ResourceBlobReader : public ResourceReader {
 protected:
  vector<unsigned char> buffer;
  bool populateBuffer();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceBlobReader(string filename);
};

/***
 * @class ResourceBlobPackedReader
 * @brief Resource reader for binary files (limited to 16k)
 * @note Resource will be saved compressed by pletter
 * @remark
 *   BLOB PACKED resource structure:
 *     blobPackedData C(blobPackedSize) - compressed by pletter
 * @todo NOT IMPLEMENTED YET
 */
class ResourceBlobPackedReader : public ResourceBlobReader {
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceBlobPackedReader(string filename);
};

/***
 * @class ResourceBlobChunkPackedReader
 * @brief Resource reader for binary files (limited to 16k)
 * @note Resource will be saved compressed by pletter in blocks of 256 bytes (1
 * byte size followed by packed data)
 * @remark
 *   BLOB CHUNK PACKED resource structure:
 *     blockCount N(2)
 *     blockList:
 *       blockSize N(1)
 *       blockData C(blockSize) - compressed by pletter
 * @todo NOT IMPLEMENTED YET
 */
class ResourceBlobChunkPackedReader : public ResourceBlobPackedReader {
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceBlobChunkPackedReader(string filename);
};

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
 protected:
  vector<string> lines;
  bool populateLines();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceTxtReader(string filename);
};

/***
 * @class ResourceCsvReader
 * @brief Resource reader for CSV files
 * @remark
 *   CSV resource structure:
 *     resourceType N(1) = 1
 *     lineCount N(2)
 *     linesMap:
 *       lineFieldCount N(1)
 *     lineList:
 *       fieldList:
 *         fieldSize N(1)
 *         fieldData C(fieldSize)
 */
class ResourceCsvReader : public ResourceTxtReader {
 protected:
  vector<string> fields;
  bool parseFields(string line);
  string fixField(string field);

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceCsvReader(string filename);
};

/***
 * @class ResourceScrReader
 * @brief Resource reader for SCn files (BLOAD binary screens)
 * @note https://msx.jannone.org/conv/
 * @remark
 *   SCn resource structure:
 *     blockCount N(2)
 *     blockList:
 *       blockSize N(1)
 *       blockData C(blockSize) - compressed by pletter
 */
class ResourceScrReader : public ResourceBlobChunkPackedReader {
 public:
  static bool isIt(string fileext);
  // bool load(); <- inherited from base class
  ResourceScrReader(string filename);
};

/***
 * @class ResourceSprReader
 * @brief Resource reader for SPR files (TinySprite plain text files)
 * @note https://msx.jannone.org/tinysprite/tinysprite.html
 * @remark
 *   SPR resource structure:
 *     blockData C(blockSize) - compressed by pletter
 * @todo NOT IMPLEMENTED YET
 */
class ResourceSprReader : public ResourceBlobPackedReader {
 private:
  bool parseTinySpriteFile();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceSprReader(string filename);

  /// @todo transfer it to parseTinySpriteFile method to be called by load()
  static int ParseTinySpriteFile(string filename, unsigned char *data,
                                 int maxlen);
};

/***
 * @class ResourceAkmReader
 * @brief Resource reader for Arkos Tracker minimalist player music files
 * (.AKM)
 * @note https://julien-nevo.com/at3test/index.php/download/
 * @todo NOT IMPLEMENTED YET
 */
class ResourceAkmReader : public ResourceBlobReader {
 private:
  void fix();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceAkmReader(string filename);

  /// @todo transfer it to fix() method to be called by load()
  static void fixAKM(unsigned char *data, int address, int length);
};

/***
 * @class ResourceAkxReader
 * @brief Resource reader for Arkos Tracker sound effects files (.AKX)
 * @note https://julien-nevo.com/at3test/index.php/download/
 * @todo NOT IMPLEMENTED YET
 */
class ResourceAkxReader : public ResourceBlobReader {
 private:
  void fix();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceAkxReader(string filename);

  /// @todo transfer it to fix() method to be called by load()
  static void fixAKX(unsigned char *data, int address, int length);
};

/***
 * @class ResourceMtfReader
 * @brief Resource reader for MSX Tile Forge projects (.mtf.json)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @todo NOT IMPLEMENTED YET
 */
class ResourceMtfReader : public ResourceBlobReader {
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceMtfReader(string filename);
};

/***
 * @class ResourceStringReader
 * @brief Resource string reader (constant string)
 * @remark
 *   String resource format:
 *     string C[stringSize]
 *     endOfString N(1) = 0
 * @todo buildMapAndResourcesText substitute
 */
class ResourceStringReader : public ResourceReader {
 public:
  bool load();
  ResourceStringReader(string text);
};

/***
 * @class ResourceDataReader
 * @brief Resource reader for DATA statements
 * @remark
 *   DATA resource structure:
 *     resourceType N(1) = 0 for DATA and 3 for IDATA
 *     lineCount N(2)
 *     linesMap:
 *       lineNumber N(2)
 *       lineFieldCount N(1)
 *     lineList:
 *       fieldList:
 *         fieldSize N(1)
 *         fieldData C(fieldSize)
 * @todo NOT IMPLEMENTED YET
 */
class ResourceDataReader : public ResourceCsvReader {
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