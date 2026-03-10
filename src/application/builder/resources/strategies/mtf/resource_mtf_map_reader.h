/***
 * @file resource_mtf_map_reader.h
 * @brief Resource MTF map reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_MTF_MAP_READER_H_INCLUDED
#define RESOURCE_MTF_MAP_READER_H_INCLUDED

#include "resource_reader.h"

/***
 * @class ResourceMtfMapReader
 * @brief Resource reader for MSX Tile Forge palette file (.SC4Map)
 * @note https://github.com/DamnedAngel/msx-tile-forge
 * @remarks
 * Resource structure
 *   BYTE resourceType = 2
 *   WORD tilemapWidth
 *   WORD tilemapHeight
 *   GROUP linesTable[tilemapHeight]
 *     BYTE lineSegment
 *     WORD lineAddress
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

#endif  // RESOURCE_MTF_MAP_READER_H_INCLUDED
