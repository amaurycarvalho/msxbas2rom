/***
 * @file symbol_output_sink.h
 * @brief Application symbol-output sink port
 * @author Amaury Carvalho (2026)
 */

#ifndef SYMBOL_OUTPUT_SINK_H_INCLUDED
#define SYMBOL_OUTPUT_SINK_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class SymbolOutputSink {
 public:
  virtual ~SymbolOutputSink() {}

  virtual bool writeText(const std::string& filename,
                         const std::string& content) = 0;
  virtual bool writeBinary(const std::string& filename,
                           const std::vector<unsigned char>& content) = 0;
};

#endif  // SYMBOL_OUTPUT_SINK_H_INCLUDED
