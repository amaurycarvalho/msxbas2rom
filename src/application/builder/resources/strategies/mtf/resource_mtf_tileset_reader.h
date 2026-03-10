/***
 * @file resource_mtf_tileset_reader.h
 * @brief Resource MTF tileset reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_MTF_TILESET_READER_H_INCLUDED
#define RESOURCE_MTF_TILESET_READER_H_INCLUDED

#include "resource_blob_reader.h"

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

#endif  // RESOURCE_MTF_TILESET_READER_H_INCLUDED
