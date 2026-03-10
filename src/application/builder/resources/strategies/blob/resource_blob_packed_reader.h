/***
 * @file resource_blob_packed_reader.h
 * @brief Resource blob packed reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_BLOB_PACKED_READER_H_INCLUDED
#define RESOURCE_BLOB_PACKED_READER_H_INCLUDED

#include "pletter.h"
#include "resource_blob_reader.h"

/***
 * @class ResourceBlobPackedReader
 * @brief Resource reader for binary files
 * @note Resource will be saved compressed by pletter
 * @remark
 *   BLOB PACKED resource structure:
 *     blobPackedData C(blobPackedSize) - compressed by pletter
 */
class ResourceBlobPackedReader : public ResourceBlobReader {
 protected:
  Pletter pletter;
  bool pack();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceBlobPackedReader(string filename);
};

#endif  // RESOURCE_BLOB_PACKED_READER_H_INCLUDED
