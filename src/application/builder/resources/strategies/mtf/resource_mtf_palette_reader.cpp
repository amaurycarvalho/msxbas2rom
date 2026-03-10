/***
 * @file resource_mtf_palette_reader.cpp
 * @brief Resource MTF palette reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_mtf_palette_reader.h"

#include <strings.h>

/***
 * @remarks
 * Palette File (.SC4Pal)
 *    Reserved Bytes (4 bytes): For future use.
 *    Color Data (48 bytes total):
 *      A sequence of 16 color entries (3 bytes each: R, G, B, with values 0-7
 *      per channel).
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
bool ResourceMtfPaletteReader::load() {
  if (ResourceBlobReader::load()) {
    /// resource type = 0 (palette)
    data[0].emplace(data[0].begin(), 1);
    data[0][0] = 0;
    packedSize = unpackedSize = data[0].size();
    return true;
  }
  return false;
}

ResourceMtfPaletteReader::ResourceMtfPaletteReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceMtfPaletteReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC4Pal") == 0);
}
