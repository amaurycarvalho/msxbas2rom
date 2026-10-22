/***
 * @file in_memory_rom_writer.h
 * @brief In-memory ROM writer for tests
 * @author Amaury Carvalho (2026)
 */

#ifndef IN_MEMORY_ROM_WRITER_H_INCLUDED
#define IN_MEMORY_ROM_WRITER_H_INCLUDED

#include <string>

#include "rom_artifact.h"
#include "rom_writer.h"

class InMemoryRomWriter : public RomWriter {
 private:
  RomArtifact lastArtifact;
  std::string lastFilename;
  bool written;

 public:
  InMemoryRomWriter() : written(false) {}

  bool write(const RomArtifact& artifact, const std::string& filename,
             std::string& error) override;

  bool hasArtifact() const;
  const RomArtifact& artifact() const;
  const std::string& filename() const;
};

#endif  // IN_MEMORY_ROM_WRITER_H_INCLUDED
