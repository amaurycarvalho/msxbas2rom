/***
 * @file file_source_resolver.h
 * @brief File-backed source resolver adapter
 * @author Amaury Carvalho (2026)
 */

#ifndef FILE_SOURCE_RESOLVER_H_INCLUDED
#define FILE_SOURCE_RESOLVER_H_INCLUDED

#include <string>

#include "source_resolver.h"

class FileSourceResolver : public SourceResolver {
 private:
  std::string error;

  bool validateHeader(const std::string& name, const std::string& content);

 public:
  FileSourceResolver() {}

  bool resolve(const std::string& name, SourceDocument& document) override;
  std::string lastError() const override;
};

#endif  // FILE_SOURCE_RESOLVER_H_INCLUDED
