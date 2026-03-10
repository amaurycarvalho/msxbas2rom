/***
 * @file resource_mtf_reader.h
 * @brief Resource MTF reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_MTF_READER_H_INCLUDED
#define RESOURCE_MTF_READER_H_INCLUDED

#include "resource_reader.h"

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

#endif  // RESOURCE_MTF_READER_H_INCLUDED
