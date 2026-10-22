/***
 * @file in_memory_symbol_output_sink.cpp
 * @brief In-memory symbol-output sink implementation
 * @author Amaury Carvalho (2026)
 */

#include "in_memory_symbol_output_sink.h"

bool InMemorySymbolOutputSink::writeText(const std::string& filename,
                                         const std::string& content) {
  textFiles[filename] = content;
  return true;
}

bool InMemorySymbolOutputSink::writeBinary(
    const std::string& filename, const std::vector<unsigned char>& content) {
  binaryFiles[filename] = content;
  return true;
}

bool InMemorySymbolOutputSink::hasText(const std::string& filename) const {
  return textFiles.find(filename) != textFiles.end();
}

bool InMemorySymbolOutputSink::hasBinary(const std::string& filename) const {
  return binaryFiles.find(filename) != binaryFiles.end();
}

std::string InMemorySymbolOutputSink::text(const std::string& filename) const {
  auto it = textFiles.find(filename);
  return it == textFiles.end() ? std::string() : it->second;
}

std::vector<unsigned char> InMemorySymbolOutputSink::binary(
    const std::string& filename) const {
  auto it = binaryFiles.find(filename);
  return it == binaryFiles.end() ? std::vector<unsigned char>() : it->second;
}
