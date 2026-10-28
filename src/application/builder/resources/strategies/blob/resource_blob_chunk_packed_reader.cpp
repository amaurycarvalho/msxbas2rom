/***
 * @file resource_blob_chunk_packed_reader.cpp
 * @brief Resource blob chunk packed reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_blob_chunk_packed_reader.h"

#include <string.h>

#include <algorithm>

#include "logger.h"

namespace {

constexpr int kChunkSize = 200;
static_assert(Pletter::maxPackedSize(kChunkSize) <= 255,
              "chunk compression must fit the 1-byte block size");

}  // namespace

ResourceBlobChunkPackedReader::ResourceBlobChunkPackedReader(
    string filename, shared_ptr<ResourceContentSource> contentSource)
    : ResourceBlobPackedReader(filename, contentSource) {
  /// @todo Refactor it implementing resource block linked list
  has1stBlockAnd2ndBlockSegmentDisalignmentBug = true;
};

bool ResourceBlobChunkPackedReader::isIt(string fileext) {
  return true;
}

/// @todo fix 1st block segment disalignment bug
/// implementing resource block linked list
bool ResourceBlobChunkPackedReader::load() {
  std::vector<unsigned char> packed;
  unsigned char* srcBuf;
  int bytesPacked, bytesUnpacked;
  int srcSize, blockCount = 0;
  if (ResourceBlobReader::load()) {
    packedSize = 0;
    isPacked = true;
    srcBuf = data[0].data();
    srcSize = data[0].size();
    while (srcSize) {
      bytesUnpacked = min(srcSize, kChunkSize);
      bytesPacked = pletter.pack(srcBuf, bytesUnpacked, packed);
      if (bytesPacked <= 0) {
        logger->error("Error while packing resource file with pletter: " +
                      filename);
        return false;
      }
      if (packed.size() > 255) {
        logger->error(
            "Block size > 255 bytes (#" + std::to_string(blockCount) +
            ") while packing resource file with pletter: " + filename);
        return false;
      }
      srcBuf += bytesUnpacked;
      srcSize -= bytesUnpacked;
      blockCount++;
      data.emplace_back(packed.size() + 1);  // create new block
      data.back()[0] = static_cast<unsigned char>(packed.size());
      memcpy(data.back().data() + 1, packed.data(), packed.size());
      packedSize += bytesPacked + 1;
    }
    data[0].resize(2);  //! block count
    data[0][0] = blockCount & 0xFF;
    data[0][1] = (blockCount >> 8) & 0xFF;
    packedSize += 2;
    return true;
  }
  return false;
}
