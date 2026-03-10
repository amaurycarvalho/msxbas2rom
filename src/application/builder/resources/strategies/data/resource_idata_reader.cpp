/***
 * @file resource_idata_reader.cpp
 * @brief Resource IDATA reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_idata_reader.h"

ResourceIDataReader::ResourceIDataReader(Parser *parser)
    : ResourceDataReader(parser) {
  filename = "_IDATA_";
  resourceType = 3;  //! IDATA resource type
  isIntegerData = true;
};
