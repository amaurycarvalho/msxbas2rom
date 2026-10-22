/***
 * @file file_symbol_output_sink.cpp
 * @brief File-backed symbol-output adapter implementation
 * @author Amaury Carvalho (2026)
 */

#include "file_symbol_output_sink.h"

#include <fstream>

bool FileSymbolOutputSink::writeText(const std::string& filename,
                                     const std::string& content) {
  std::ofstream file(filename.c_str(), std::ios::binary);
  if (!file) return false;
  file.write(content.data(), content.size());
  return file.good();
}

bool FileSymbolOutputSink::writeBinary(
    const std::string& filename, const std::vector<unsigned char>& content) {
  std::ofstream file(filename.c_str(), std::ios::binary);
  if (!file) return false;
  if (!content.empty()) {
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
  }
  return file.good();
}
