/***
 * @file rom_writer.h
 * @brief Application ROM persistence port
 * @author Amaury Carvalho (2026)
 */

#ifndef ROM_WRITER_H_INCLUDED
#define ROM_WRITER_H_INCLUDED

#include <memory>
#include <string>

class RomArtifact;

class RomWriter {
 public:
  virtual ~RomWriter() {}

  virtual bool write(const RomArtifact& artifact, const std::string& filename,
                     std::string& error) = 0;
};

#endif  // ROM_WRITER_H_INCLUDED
