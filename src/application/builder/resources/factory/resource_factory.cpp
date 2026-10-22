/***
 * @file resource_factory.cpp
 * @brief Resource factory implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_factory.h"

#include "build_path_policy.h"
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

std::shared_ptr<ResourceReader> ResourceFactory::create(string filename) {
  string fileext = build_path::getFileExtension(filename);
  if (ResourceTxtReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceTxtReader(filename, contentSource));
  }
  if (ResourceCsvReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceCsvReader(filename, contentSource));
  }
  if (ResourceScrReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceScrReader(filename, contentSource));
  }
  if (ResourceSprReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceSprReader(filename, contentSource));
  }
  if (ResourceAkmReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceAkmReader(filename, contentSource));
  }
  if (ResourceAkxReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceAkxReader(filename, contentSource));
  }
  if (ResourceMtfReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceMtfReader(filename, contentSource));
  }
  if (ResourceMtfPaletteReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(
        new ResourceMtfPaletteReader(filename, contentSource));
  }
  if (ResourceMtfTilesetReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(
        new ResourceMtfTilesetReader(filename, contentSource));
  }
  if (ResourceMtfMapReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceMtfMapReader(filename, contentSource));
  }
  if (ResourceBlobReader::isIt(fileext)) {
    return std::shared_ptr<ResourceReader>(new ResourceBlobReader(filename, contentSource));
  }
  return nullptr;
}

ResourceFactory::ResourceFactory(
    std::shared_ptr<ResourceContentSource> source)
    : contentSource(source) {}

ResourceFactory::~ResourceFactory() = default;
