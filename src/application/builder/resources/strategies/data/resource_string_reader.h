/***
 * @file resource_string_reader.h
 * @brief Resource string reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_STRING_READER_H_INCLUDED
#define RESOURCE_STRING_READER_H_INCLUDED

#include "resource_reader.h"

/***
 * @class ResourceStringReader
 * @brief Resource string reader (constant string)
 * @remark
 *   String resource format:
 *     string C[stringSize]
 *     endOfString N(1) = 0
 */
class ResourceStringReader : public ResourceReader {
 public:
  bool load();
  ResourceStringReader(string text);
};

#endif  // RESOURCE_STRING_READER_H_INCLUDED
