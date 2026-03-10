/***
 * @file resource_scr_reader.cpp
 * @brief Resource SCR reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_scr_reader.h"

#include <strings.h>

ResourceScrReader::ResourceScrReader(string filename)
    : ResourceBlobChunkPackedReader(filename) {};

bool ResourceScrReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC0") == 0 ||
          strcasecmp(fileext.c_str(), ".SC1") == 0 ||
          strcasecmp(fileext.c_str(), ".SC2") == 0 ||
          strcasecmp(fileext.c_str(), ".SC3") == 0 ||
          strcasecmp(fileext.c_str(), ".SC4") == 0 ||
          strcasecmp(fileext.c_str(), ".SC5") == 0 ||
          strcasecmp(fileext.c_str(), ".SC6") == 0 ||
          strcasecmp(fileext.c_str(), ".SC7") == 0 ||
          strcasecmp(fileext.c_str(), ".SC8") == 0 ||
          strcasecmp(fileext.c_str(), ".SC9") == 0 ||
          strcasecmp(fileext.c_str(), ".S10") == 0 ||
          strcasecmp(fileext.c_str(), ".S11") == 0 ||
          strcasecmp(fileext.c_str(), ".S12") == 0);
}

/* inherited from base class
bool ResourceScrReader::load() {
  return false;
}
*/
