/***
 * @file resource_string_reader.cpp
 * @brief Resource string reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_string_reader.h"

#include <algorithm>
#include <string.h>

ResourceStringReader::ResourceStringReader(string text)
    : ResourceReader(text) {};

bool ResourceStringReader::load() {
  int lineSize;

  data.clear();

  /// string maximum size = 255 chars
  lineSize = min((int)filename.size(), 255);
  /// set string size
  data.emplace_back(lineSize + 1);
  /// copy string text (max = 255 chars)
  memcpy(data.back().data(), filename.c_str(), lineSize);
  /// end of line
  data.back()[lineSize] = 0;

  unpackedSize = lineSize + 1;
  packedSize = lineSize + 1;

  return true;
}
