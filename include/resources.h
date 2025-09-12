/***
 * @file resources.h
 * @brief Resources class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

#include <fstream>

#include "symbols.h"

/***
 * @class ResourceReader
 * @brief Resource reader base class (abstract)
 */
class ResourceReader {
 protected:
  string filename;
  string errorMessage;

 public:
  vector<vector<unsigned char>> data;
  int unpackedSize;
  int packedSize;
  const string getErrorMessage();
  const string getFilename();
  bool remapTo(int index, int mappedSegm, int mappedAddress);
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
 */
class ResourceBlobReader : public ResourceReader {
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceBlobReader(string filename);
};

/***
 * @class ResourceBlobPackedReader
 * @brief Resource reader for binary files
 * @note Resource will be saved compressed by pletter
 * @remark
 *   BLOB PACKED resource structure:
 *     blobPackedData C(blobPackedSize) - compressed by pletter
 */
class ResourceBlobPackedReader : public ResourceBlobReader {
 protected:
  bool pack();

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

 public:
  static bool isIt(string fileext);
  bool populateLines();
  const vector<string> getLines();
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
  int resourceType;
  vector<int> lineNumbers;
  vector<vector<string>> lineFields;

  void addFields(string line);
  string fixFieldValue(string field);

  bool populateFields();
  bool populateData();

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
 */
class ResourceSprReader : public ResourceBlobPackedReader {
 private:
  bool parseTinySpriteFile();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceSprReader(string filename);

  /// @todo remove this old piece of code (deprecated)
  static int ParseTinySpriteFile(string filename, unsigned char *data,
                                 int maxlen);
};

/***
 * @class ResourceAkmReader
 * @brief Resource reader for Arkos Tracker minimalist player music files
 * (.AKM)
 * @note https://julien-nevo.com/at3test/index.php/download/
 * @todo fixAKM function refactoring
 */
class ResourceAkmReader : public ResourceBlobReader {
 public:
  static bool isIt(string fileext);
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceAkmReader(string filename);

  /// @todo tranform it from static function to a private method
  static void fixAKM(unsigned char *data, int address, int length);
};

/***
 * @class ResourceAkxReader
 * @brief Resource reader for Arkos Tracker sound effects files (.AKX)
 * @note https://julien-nevo.com/at3test/index.php/download/
 * @todo fixAKX function refactoring
 */
class ResourceAkxReader : public ResourceBlobReader {
 public:
  static bool isIt(string fileext);
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceAkxReader(string filename);

  /// @todo tranform it from static function to a private method
  static void fixAKX(unsigned char *data, int address, int length);
};

/***
 * @class ResourceMtfReader
 * @brief Resource reader for MSX Tile Forge projects (.mtf.json)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @remarks
 * Resource structure
 *   Resource Map
 *     BYTE palleteSegment
 *     WORD palleteAddress
 *     BYTE tilesetSegment
 *     WORD tilesetAddress
 *     BYTE tilemapSegment
 *     WORD tilemapAddress
 *   Pallete Data (.SC4Pal)
 *     Color Data
 *   Tileset Data (.SC4Tiles, formated to screen 2/4)
 *     All Pattern Data Block and All Color Attribute Data Block
 *   Tilemap Header (.SC4Super and .SC4Map)
 *     WORD tilemapWidth
 *     WORD tilemapHeight
 *     GROUP tilemapLinesAddresses[tilemapHeight]
 *       BYTE tilemapLineSegment
 *       WORD tilemapLineAddress
 *   Tilemap Line Data [tilemapHeight] (formatted to screen 2/4)
 *     BYTE tileNumber[tilemapWidth]
 */
/// @todo NOT IMPLEMENTED YET
class ResourceMtfReader : public ResourceBlobReader {
 private:
  int palleteSegment, palleteAddress;
  int tilesetSegment, tilesetAddress;
  int tilemapSegment, tilemapAddress;
  int tilemapWidth, tilemapHeight;

 public:
  static bool isIt(string fileext);
  bool load();
  bool remapTo(int index, int mappedSegm, int mappedAddress);
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
 */
class ResourceDataReader : public ResourceCsvReader {
 protected:
  bool isIntegerData;

 private:
  Parser *parser;
  bool populateFields();

 public:
  bool load();
  ResourceDataReader(Parser *parser);
};

/***
 * @class ResourceIDataReader
 * @brief Resource reader for IDATA statements
 * @remark
 *   IDATA resource structure:
 *     resourceType N(1) = 0 for DATA and 3 for IDATA
 *     lineCount N(2)
 *     linesMap:
 *       lineNumber N(2)
 *       lineFieldCount N(1)
 *     lineList:
 *       fieldList:
 *         fieldData N(2)
 */
class ResourceIDataReader : public ResourceDataReader {
 public:
  ResourceIDataReader(Parser *parser);
};

/***
 * @class ResourceManager
 * @brief Resource manager
 */
class ResourceManager {
 private:
  string errorMessage;

 public:
  vector<ResourceReader *> resources;
  vector<vector<unsigned char>> pages;

  int resourcesPackedSize;
  int resourcesUnpackedSize;
  float packedRate;

  /***
   * @brief clear all resources
   */
  void clear();

  /***
   * @brief add a new FILE resource
   */
  bool addFile(string filename, string inputPath);

  /***
   * @brief Add a new TEXT string resource to the resource list
   */
  void addText(string text);

  /***
   * @brief Add a DATA statement resource to the resource list
   * @todo Implement IDATA+DATA mix up bug fix
   */
  void addDataResource(Parser *parser);

  /***
   * @brief Add a IDATA statement resource to the resource list
   * @todo Implement IDATA+DATA mix up bug fix
   */
  void addIDataResource(Parser *parser);

  /***
   * @brief build map and resources data
   * @remarks
   * Resources location on ROM:
   *    48kb ROM - starts on page 0
   *    128kb MEGAROM - starts on the next segment after the code
   * Resources are structured in map and data sections
   *    map section starts at position 0x0010 of first segment
   *       WORD resource_count
   *       struct resource {
   *         WORD offset_on_page,
   *         BYTE segment,
   *         WORD resource_size
   *        } [resource_count]
   *       so, a max of 48 resources is allowed (=5 * 48 + 16 = 256 bytes)
   *    data section starts at position 0x0100 of first segment
   *       TEXT:
   *         same as resource type TXT of DATA
   *       DATA:
   *         BYTE data_resource_type
   *         resource type 0 - DATA
   *            WORD resource_items_count
   *            struct lines_map { WORD line_number, BYTE line_items_count }
   *            [resource_items_count] struct lines_data { STRINGS
   *            item_data[line_items_count] } [resource_items_count]
   *         resource type 1 - CSV
   *            WORD resource_items_count
   *            struct lines_map { BYTE line_items_count }
   *            [resource_items_count] struct lines_data { STRINGS
   *            item_data[line_items_count] } [resource_items_count]
   *         resource type 2 - TXT
   *            STRINGS item_data[]
   *         resource type 3 - IDATA
   *            WORD resource_items_count
   *            struct lines_map { WORD line_number, BYTE line_items_count }
   *            [resource_items_count] struct lines_data { WORD
   *            item_data[line_items_count] } [resource_items_count]
   *       FILE:
   *         ARKOS files - plain data, continuous on the same segment
   *            BYTE data[]
   *         NMSXTILES files - compressed, continuous on the same segment or
   *         move to the next
   *            BYTE data[]
   *         TINYSPRITE files - compressed, continuous on the same segment or
   *         move to the next
   *            BYTE data[]
   *         SCREEN IMAGE (BLOAD) files - compressed, discontinuous (blocks of
   *         256 bytes)
   *            WORD blocks, STRINGS data[]
   *                         ^== each block starts with its compressed data
   *                         length (1 byte)
   *                             followed by the compressed data itself
   */
  bool buildMap(int startSegment, int startAddress);

  /***
   * @brief return last error message
   */
  const string getErrorMessage();

  /***
   * @brief print resources names
   */
  void print();
};

#endif  // RESOURCES_H_INCLUDED