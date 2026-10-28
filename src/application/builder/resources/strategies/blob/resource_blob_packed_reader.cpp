/***
 * @file resource_blob_packed_reader.cpp
 * @brief Resource blob packed reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_blob_packed_reader.h"

#include "logger.h"

ResourceBlobPackedReader::ResourceBlobPackedReader(
    string filename, shared_ptr<ResourceContentSource> contentSource)
    : ResourceBlobReader(filename, contentSource) {};

bool ResourceBlobPackedReader::pack() {
  int bytesPacked = 0;
  if (data.empty() || data[0].empty()) {
    logger->error(
        "Resource file is empty so it's no possible to pack it with pletter: " +
        filename);
    return false;
  }
  unpackedSize = data[0].size();
  packedSize = unpackedSize;
  isPacked = true;
  if (unpackedSize > 0x2000) {
    logger->error(
        "Resource size > 8k so it's no possible to unpack it\non machines with "
        "limited RAM: " +
        filename);
    return false;
  }
  std::vector<unsigned char> packed;
  bytesPacked =
      pletter.pack(data[0].data(), static_cast<int>(data[0].size()), packed);
  if (!bytesPacked) {
    logger->error("Error while packing resource file with pletter: " +
                  filename);
    return false;
  }
  data[0].swap(packed);  //! replaces the uncompressed data
  packedSize = bytesPacked;
  return true;
}

bool ResourceBlobPackedReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobPackedReader::load() {
  if (ResourceBlobReader::load()) {
    return pack();
  }
  return false;
}
