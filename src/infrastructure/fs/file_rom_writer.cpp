/***
 * @file file_rom_writer.cpp
 * @brief File-backed ROM writer adapter implementation
 * @author Amaury Carvalho (2026)
 */

#include "file_rom_writer.h"

#include <cstdio>
#include <fstream>

#include "rom_artifact.h"

bool FileRomWriter::write(const RomArtifact& artifact,
                          const std::string& filename, std::string& error) {
  std::ofstream file(filename.c_str(), std::ios::binary);
  if (!file) {
    error = "Cannot create output file: " + filename;
    return false;
  }

  for (std::size_t i = 0; i < artifact.pages.size(); i++) {
    file.write(reinterpret_cast<const char*>(artifact.pages[i].data()),
               artifact.pages[i].size());
    if (!file) {
      error = "Cannot write output file: " + filename;
      file.close();
      std::remove(filename.c_str());
      return false;
    }
  }

  file.close();
  error.clear();
  return true;
}
