/***
 * @file resource_idata_reader.h
 * @brief Resource IDATA reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_IDATA_READER_H_INCLUDED
#define RESOURCE_IDATA_READER_H_INCLUDED

#include "resource_data_reader.h"

/***
 * @class ResourceIDataReader
 * @brief Resource reader for IDATA statements
 * @remark
 *   IDATA resource structure:
 *     resourceType N(1) = 0 for DATA and 3 for IDATA
 *     lineCount N(2)
 *     linesMap:
 *       lineNumber N(2)
 *       lineFieldCount N(1)
 *     lineList:
 *       fieldList:
 *         fieldData N(2)
 */
class ResourceIDataReader : public ResourceDataReader {
 public:
  ResourceIDataReader(shared_ptr<Parser> parser);
};

#endif  // RESOURCE_IDATA_READER_H_INCLUDED
