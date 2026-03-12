/***
 * @file resource_blob_reader.cpp
 * @brief Resource blob reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_blob_reader.h"

#include <fstream>

#include "logger.h"

ResourceBlobReader::ResourceBlobReader(string filename)
    : ResourceReader(filename) {};

bool ResourceBlobReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobReader::load() {
  ifstream fileStream(filename, ios::binary | ios::ate);

  data.clear();
  unpackedSize = 0;
  packedSize = 0;

  if (!fileStream) {
    logger->error("Resource file not found: " + filename);
    return false;
  }

  streamsize size = fileStream.tellg();
  fileStream.seekg(0, ios::beg);

  if (size <= 0) {
    logger->error("Resource is empty: " + filename);
    return false;
  }

  data.emplace_back(size);
  if (!fileStream.read((char*)data.back().data(), size)) {
    logger->error("Error reading resource: " + filename);
    return false;
  }

  unpackedSize = size;
  packedSize = size;

  return true;
}
