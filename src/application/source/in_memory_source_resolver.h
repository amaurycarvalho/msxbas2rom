/***
 * @file in_memory_source_resolver.h
 * @brief In-memory source resolver for tests and embedded programs
 * @author Amaury Carvalho (2026)
 */

#ifndef IN_MEMORY_SOURCE_RESOLVER_H_INCLUDED
#define IN_MEMORY_SOURCE_RESOLVER_H_INCLUDED

#include <map>
#include <string>

#include "source_resolver.h"

class InMemorySourceResolver : public SourceResolver {
 private:
  std::map<std::string, std::string> sources;
  std::string error;

 public:
  InMemorySourceResolver() {}

  void add(const std::string& name, const std::string& content);
  void clear();
  bool resolve(const std::string& name, SourceDocument& document) override;
  std::string lastError() const override;
};

#endif  // IN_MEMORY_SOURCE_RESOLVER_H_INCLUDED
