/***
 * @file in_memory_rom_writer.cpp
 * @brief In-memory ROM writer implementation
 * @author Amaury Carvalho (2026)
 */

#include "in_memory_rom_writer.h"

bool InMemoryRomWriter::write(const RomArtifact& artifact,
                              const std::string& filename,
                              std::string& error) {
  lastArtifact = artifact;
  lastFilename = filename;
  written = true;
  error.clear();
  return true;
}

bool InMemoryRomWriter::hasArtifact() const {
  return written;
}

const RomArtifact& InMemoryRomWriter::artifact() const {
  return lastArtifact;
}

const std::string& InMemoryRomWriter::filename() const {
  return lastFilename;
}
