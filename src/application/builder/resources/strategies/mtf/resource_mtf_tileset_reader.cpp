/***
 * @file resource_mtf_tileset_reader.cpp
 * @brief Resource MTF tileset reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_mtf_tileset_reader.h"

#include <strings.h>

/***
 * @remarks
 * Tileset File (.SC4Tiles)
 *    Header:
 *      num_tiles_in_file(1 byte): A value of 0 indicates 256 tiles.
 *      Project Tile Limit (1 byte): A value of 0 indicates 256 tiles.
 *      Reserved Bytes (3 bytes): Currently unused.
 *    All Pattern Data Block (Total: num_tiles * 8 bytes):
 *      Pattern data for all tiles, stored consecutively. Each tile is 8 bytes
 *      (1 byte per row). In each byte, the most significant bit is the
 *      leftmost pixel.
 *    All Color Attribute Data Block (Total: num_tiles * 8 bytes):
 *      Color attribute data for all tiles, stored consecutively. Each
 *      tile is 8 bytes (1 byte per row). The high nibble (4 bits) is the
 *      foreground palette index, and the low nibble is the background palette
 *      index.
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
bool ResourceMtfTilesetReader::load() {
  if (ResourceBlobReader::load()) {
    /// resource type = 1 (tileset)
    data[0].emplace(data[0].begin(), 1);
    data[0][0] = 1;
    packedSize = unpackedSize = data[0].size();
    return true;
  }
  return false;
}

ResourceMtfTilesetReader::ResourceMtfTilesetReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceMtfTilesetReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC4Tiles") == 0);
}
