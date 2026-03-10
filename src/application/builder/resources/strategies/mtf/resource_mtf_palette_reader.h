/***
 * @file resource_mtf_palette_reader.h
 * @brief Resource MTF palette reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_MTF_PALETTE_READER_H_INCLUDED
#define RESOURCE_MTF_PALETTE_READER_H_INCLUDED

#include "resource_blob_reader.h"

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

#endif  // RESOURCE_MTF_PALETTE_READER_H_INCLUDED
