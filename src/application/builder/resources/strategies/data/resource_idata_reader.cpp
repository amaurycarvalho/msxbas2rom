/***
 * @file resource_idata_reader.cpp
 * @brief Resource IDATA reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_idata_reader.h"

#include "parser.h"

ResourceIDataReader::ResourceIDataReader(shared_ptr<Parser> parser)
    : ResourceDataReader(parser) {
  filename = "_IDATA_";
  resourceType = 3;  //! IDATA resource type
  isIntegerData = true;
};
