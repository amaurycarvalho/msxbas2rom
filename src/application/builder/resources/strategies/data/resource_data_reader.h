/***
 * @file resource_data_reader.h
 * @brief Resource DATA reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_DATA_READER_H_INCLUDED
#define RESOURCE_DATA_READER_H_INCLUDED

#include "resource_csv_reader.h"

class Parser;

/***
 * @class ResourceDataReader
 * @brief Resource reader for DATA statements
 * @remark
 *   DATA resource structure:
 *     resourceType N(1) = 0 for DATA and 3 for IDATA
 *     lineCount N(2)
 *     linesMap:
 *       lineNumber N(2)
 *       lineFieldCount N(1)
 *     lineList:
 *       fieldList:
 *         fieldSize N(1)
 *         fieldData C(fieldSize)
 */
class ResourceDataReader : public ResourceCsvReader {
 private:
  shared_ptr<Parser> parser;
  bool populateFields();

 public:
  bool load();
  ResourceDataReader(shared_ptr<Parser> parser);
};

#endif  // RESOURCE_DATA_READER_H_INCLUDED
