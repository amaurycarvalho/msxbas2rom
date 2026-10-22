/***
 * @file in_memory_symbol_output_sink.h
 * @brief In-memory symbol-output sink for tests
 * @author Amaury Carvalho (2026)
 */

#ifndef IN_MEMORY_SYMBOL_OUTPUT_SINK_H_INCLUDED
#define IN_MEMORY_SYMBOL_OUTPUT_SINK_H_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "symbol_output_sink.h"

class InMemorySymbolOutputSink : public SymbolOutputSink {
 private:
  std::map<std::string, std::string> textFiles;
  std::map<std::string, std::vector<unsigned char>> binaryFiles;

 public:
  InMemorySymbolOutputSink() {}

  bool writeText(const std::string& filename,
                 const std::string& content) override;
  bool writeBinary(const std::string& filename,
                   const std::vector<unsigned char>& content) override;

  bool hasText(const std::string& filename) const;
  bool hasBinary(const std::string& filename) const;
  std::string text(const std::string& filename) const;
  std::vector<unsigned char> binary(const std::string& filename) const;
};

#endif  // IN_MEMORY_SYMBOL_OUTPUT_SINK_H_INCLUDED
