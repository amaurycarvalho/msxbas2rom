/***
 * @file resource_scr_reader.h
 * @brief Resource SCR reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_SCR_READER_H_INCLUDED
#define RESOURCE_SCR_READER_H_INCLUDED

#include "resource_blob_chunk_packed_reader.h"

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

#endif  // RESOURCE_SCR_READER_H_INCLUDED
