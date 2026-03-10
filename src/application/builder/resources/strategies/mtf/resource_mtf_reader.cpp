/***
 * @file resource_mtf_reader.cpp
 * @brief Resource MTF reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_mtf_reader.h"

#include <strings.h>

/***
 * @remarks
 * Wrapper to ResourceMtfPalReader, ResourceMtfTilesReader and
 * ResourceMtfMapReader
 */
/// @todo NOT IMPLEMENTED YET
bool ResourceMtfReader::load() {
  errorMessage = "Not implemented yet (ResourceMtfReader::load)";
  return false;
}

ResourceMtfReader::ResourceMtfReader(string filename)
    : ResourceReader(filename) {};

bool ResourceMtfReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".MTF") == 0);
}
