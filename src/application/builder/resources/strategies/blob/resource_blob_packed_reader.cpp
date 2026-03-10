/***
 * @file resource_blob_packed_reader.cpp
 * @brief Resource blob packed reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_blob_packed_reader.h"

ResourceBlobPackedReader::ResourceBlobPackedReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceBlobPackedReader::pack() {
  int bytesPacked = 0;
  if (!data.size()) {
    errorMessage =
        "Resource file is empty so it's no possible to pack it with pletter: " +
        filename;
    return false;
  }
  unpackedSize = data[0].size();
  packedSize = unpackedSize;
  isPacked = true;
  if (unpackedSize > 0x2000) {
    errorMessage =
        "Resource size > 8k so it's no possible to unpack it\non machines with "
        "limited RAM: " +
        filename;
    return false;
  }
  data.emplace_back(data[0].size() * 2);
  bytesPacked = pletter.pack(data[0].data(), data[0].size(), data[1].data());
  if (!bytesPacked) {
    errorMessage =
        "Error while packing resource file with pletter: " + filename;
    return false;
  }
  data.erase(data.begin());     //! removes the uncompressed data
  data[0].resize(bytesPacked);  //! fix the compressed data size
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
