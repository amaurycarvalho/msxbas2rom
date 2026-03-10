/***
 * @file resource_blob_reader.h
 * @brief Resource blob reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_BLOB_READER_H_INCLUDED
#define RESOURCE_BLOB_READER_H_INCLUDED

#include "resource_reader.h"

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

#endif  // RESOURCE_BLOB_READER_H_INCLUDED
