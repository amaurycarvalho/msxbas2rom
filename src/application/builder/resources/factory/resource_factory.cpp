/***
 * @file resource_factory.cpp
 * @brief Resource factory implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_factory.h"

#include "fswrapper.h"
#include "resource_akm_reader.h"
#include "resource_akx_reader.h"
#include "resource_blob_reader.h"
#include "resource_csv_reader.h"
#include "resource_mtf_map_reader.h"
#include "resource_mtf_palette_reader.h"
#include "resource_mtf_reader.h"
#include "resource_mtf_tileset_reader.h"
#include "resource_scr_reader.h"
#include "resource_spr_reader.h"
#include "resource_txt_reader.h"

std::unique_ptr<ResourceReader> ResourceFactory::create(string filename) {
  string fileext = getFileExtension(filename);
  if (ResourceTxtReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceTxtReader(filename));
  }
  if (ResourceCsvReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceCsvReader(filename));
  }
  if (ResourceScrReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceScrReader(filename));
  }
  if (ResourceSprReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceSprReader(filename));
  }
  if (ResourceAkmReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceAkmReader(filename));
  }
  if (ResourceAkxReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceAkxReader(filename));
  }
  if (ResourceMtfReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceMtfReader(filename));
  }
  if (ResourceMtfPaletteReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(
        new ResourceMtfPaletteReader(filename));
  }
  if (ResourceMtfTilesetReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(
        new ResourceMtfTilesetReader(filename));
  }
  if (ResourceMtfMapReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceMtfMapReader(filename));
  }
  if (ResourceBlobReader::isIt(fileext)) {
    return std::unique_ptr<ResourceReader>(new ResourceBlobReader(filename));
  }
  return nullptr;
}
