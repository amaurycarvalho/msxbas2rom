/***
 * @file resource_reader.cpp
 * @brief Resource reader base class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_reader.h"

const string ResourceReader::getErrorMessage() {
  return errorMessage;
}

const string ResourceReader::getFilename() {
  return filename;
}

bool ResourceReader::remapTo(int index, int mappedSegm, int mappedAddress) {
  return true;
};

ResourceReader::ResourceReader(string filename) {
  this->filename = filename;
  this->packedSize = 0;
  this->unpackedSize = 0;
  this->isPacked = false;
  /// @todo Refactor it creating linked lists in the affected resources
  this->has1stBlockAnd2ndBlockSegmentDisalignmentBug = false;
};
