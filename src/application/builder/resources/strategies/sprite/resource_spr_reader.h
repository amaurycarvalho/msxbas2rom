/***
 * @file resource_spr_reader.h
 * @brief Resource SPR reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_SPR_READER_H_INCLUDED
#define RESOURCE_SPR_READER_H_INCLUDED

#include "resource_blob_packed_reader.h"

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

#endif  // RESOURCE_SPR_READER_H_INCLUDED
