/***
 * @file resource_blob_reader.cpp
 * @brief Resource blob reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_blob_reader.h"

#include "logger.h"
#include "resource_content_source.h"

ResourceBlobReader::ResourceBlobReader(
    string filename, shared_ptr<ResourceContentSource> contentSource)
    : ResourceReader(filename, contentSource) {};

bool ResourceBlobReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobReader::load() {
  vector<unsigned char> content;
  string error;

  data.clear();
  unpackedSize = 0;
  packedSize = 0;

  if (!contentSource ||
      !contentSource->readBinary(filename, content, error)) {
    logger->error(error.empty() ? "Resource file not found: " + filename
                                : error);
    return false;
  }

  data.push_back(content);
  unpackedSize = content.size();
  packedSize = content.size();

  return true;
}
