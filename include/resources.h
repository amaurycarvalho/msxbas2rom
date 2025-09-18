/***
 * @file resources.h
 * @brief Resources class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

#include <fstream>

#include "pletter.h"
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
  bool isPacked;
  const string getErrorMessage();
  const string getFilename();
  virtual bool remapTo(int index, int mappedSegm, int mappedAddress);
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
  Pletter pletter;
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
 public:
  vector<string> lines;
  static bool isIt(string fileext);
  bool populateLines();
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
  bool isIntegerData;
  vector<int> lineNumbers;
  vector<vector<string>> lineFields;

  void addFields(string line);
  string fixFieldValue(string field);

  virtual bool populateFields();
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
};

/***
 * @class ResourceAkmReader
 * @brief Resource reader for Arkos Tracker minimalist player music files
 * (.AKM)
 * @remarks
 * https://www.julien-nevo.com/arkostracker/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/AKM.md
 */
class ResourceAkmReader : public ResourceBlobReader {
 private:
  bool fixAKM(unsigned char *data, int address, int length);
  int guessBaseAddress(unsigned char *data, int length);

 public:
  static bool isIt(string fileext);
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceAkmReader(string filename);
};

/***
 * @class ResourceAkxReader
 * @brief Resource reader for Arkos Tracker sound effects files (.AKX)
 * @remarks
 * https://www.julien-nevo.com/arkostracker/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/SoundEffects.md
 */
class ResourceAkxReader : public ResourceBlobReader {
 private:
  bool fixAKX(unsigned char *data, int address, int length);
  int guessBaseAddress(unsigned char *data, int length);

 public:
  static bool isIt(string fileext);
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceAkxReader(string filename);
};

/***
 * @class ResourceMtfReader
 * @brief Resource reader for MSX Tile Forge projects (.mtf.json)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @remarks
 * Wrapper to ResourceMtfPalReader, ResourceMtfTilesReader and
 * ResourceMtfMapReader
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
/// @todo NOT IMPLEMENTED YET
class ResourceMtfReader : public ResourceReader {
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceMtfReader(string filename);
};

/***
 * @class ResourceMtfPaletteReader
 * @brief Resource reader for MSX Tile Forge palette file (.SC4Pal)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @remarks
 * Resource structure
 *   BYTE resourceType = 0
 *   Pallete Data (.SC4Pal)
 *     Color Data
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
class ResourceMtfPaletteReader : public ResourceBlobReader {
 private:
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceMtfPaletteReader(string filename);
};

/***
 * @class ResourceMtfTilesetReader
 * @brief Resource reader for MSX Tile Forge palette file (.SC4Tiles)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @remarks
 * Resource structure
 *   BYTE resourceType = 1
 *   Tileset Data (.SC4Tiles)
 *     All Pattern Data Block and All Color Attribute Data Block
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
class ResourceMtfTilesetReader : public ResourceBlobReader {
 private:
 public:
  static bool isIt(string fileext);
  bool load();
  ResourceMtfTilesetReader(string filename);
};

/***
 * @class ResourceMtfMapReader
 * @brief Resource reader for MSX Tile Forge palette file (.SC4Map)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @remarks
 * Resource structure
 *   BYTE resourceType = 2
 *   WORD tilemapWidth
 *   WORD tilemapHeight
 *   BYTE firstLineSegment
 *   WORD firstLineAddress
 *   Tilemap Line Data [tilemapHeight] <-- .SC4Super + .SC4Map
 *     BYTE nextLineSegment
 *     WORD nextLineAddress
 *     BYTE tilemap[tilemapWidth+31] <-- copy of first 31 tiles at end
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
class ResourceMtfMapReader : public ResourceReader {
 private:
  string supertileFilename;
  int supertileCount;
  int supertileWidth, supertileHeight;
  int supertileHeaderSkip;
  int tilemapWidth, tilemapHeight;
  int tilemapResourceWidth, tilemapResourceHeight;

 public:
  static bool isIt(string fileext);
  bool load();
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceMtfMapReader(string filename);
};

/***
 * @class ResourceStringReader
 * @brief Resource string reader (constant string)
 * @remark
 *   String resource format:
 *     string C[stringSize]
 *     endOfString N(1) = 0
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