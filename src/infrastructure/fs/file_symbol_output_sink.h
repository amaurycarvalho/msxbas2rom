/***
 * @file file_symbol_output_sink.h
 * @brief File-backed symbol-output adapter
 * @author Amaury Carvalho (2026)
 */

#ifndef FILE_SYMBOL_OUTPUT_SINK_H_INCLUDED
#define FILE_SYMBOL_OUTPUT_SINK_H_INCLUDED

#include "symbol_output_sink.h"

class FileSymbolOutputSink : public SymbolOutputSink {
 public:
  FileSymbolOutputSink() {}

  bool writeText(const std::string& filename,
                 const std::string& content) override;
  bool writeBinary(const std::string& filename,
                   const std::vector<unsigned char>& content) override;
};

#endif  // FILE_SYMBOL_OUTPUT_SINK_H_INCLUDED
