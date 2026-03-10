/***
 * @file resource_blob_chunk_packed_reader.h
 * @brief Resource blob chunk packed reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_BLOB_CHUNK_PACKED_READER_H_INCLUDED
#define RESOURCE_BLOB_CHUNK_PACKED_READER_H_INCLUDED

#include "resource_blob_packed_reader.h"

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

#endif  // RESOURCE_BLOB_CHUNK_PACKED_READER_H_INCLUDED
