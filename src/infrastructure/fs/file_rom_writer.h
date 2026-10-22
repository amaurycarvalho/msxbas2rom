/***
 * @file file_rom_writer.h
 * @brief File-backed ROM writer adapter
 * @author Amaury Carvalho (2026)
 */

#ifndef FILE_ROM_WRITER_H_INCLUDED
#define FILE_ROM_WRITER_H_INCLUDED

#include "rom_writer.h"

class FileRomWriter : public RomWriter {
 public:
  FileRomWriter() {}

  bool write(const RomArtifact& artifact, const std::string& filename,
             std::string& error) override;
};

#endif  // FILE_ROM_WRITER_H_INCLUDED
