/***
 * @file resource_reader.cpp
 * @brief Resource reader base class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_reader.h"

#include "logger.h"
#include "resource_content_source.h"

shared_ptr<Logger> ResourceReader::getLogger() {
  return logger;
}

const string ResourceReader::getFilename() {
  return filename;
}

bool ResourceReader::remapTo(int index, int mappedSegm, int mappedAddress) {
  return true;
};

ResourceReader::ResourceReader(
    string filename, shared_ptr<ResourceContentSource> source) {
  logger = make_shared<Logger>();
  contentSource = source;
  this->filename = filename;
  this->packedSize = 0;
  this->unpackedSize = 0;
  this->isPacked = false;
  /// @todo Refactor it creating linked lists in the affected resources
  this->has1stBlockAnd2ndBlockSegmentDisalignmentBug = false;
}

ResourceReader::~ResourceReader() = default;
