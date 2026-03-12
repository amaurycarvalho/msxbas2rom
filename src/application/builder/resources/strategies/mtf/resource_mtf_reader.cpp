/***
 * @file resource_mtf_reader.cpp
 * @brief Resource MTF reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_mtf_reader.h"

#include <strings.h>

#include "logger.h"

/***
 * @remarks
 * Wrapper to ResourceMtfPalReader, ResourceMtfTilesReader and
 * ResourceMtfMapReader
 */
/// @todo NOT IMPLEMENTED YET
bool ResourceMtfReader::load() {
  logger->error("Not implemented yet (ResourceMtfReader::load)");
  return false;
}

ResourceMtfReader::ResourceMtfReader(string filename)
    : ResourceReader(filename) {};

bool ResourceMtfReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".MTF") == 0);
}
