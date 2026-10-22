/***
 * @file in_memory_source_resolver.cpp
 * @brief In-memory source resolver implementation
 * @author Amaury Carvalho (2026)
 */

#include "in_memory_source_resolver.h"

void InMemorySourceResolver::add(const std::string& name,
                                 const std::string& content) {
  sources[name] = content;
}

void InMemorySourceResolver::clear() {
  sources.clear();
  error.clear();
}

bool InMemorySourceResolver::resolve(const std::string& name,
                                     SourceDocument& document) {
  auto it = sources.find(name);
  if (it == sources.end()) {
    error = "File doesn't exist";
    return false;
  }

  if (it->second.empty()) {
    error = "Empty file";
    return false;
  }
  document.logicalName = name;
  document.content = it->second;
  error.clear();
  return true;
}

std::string InMemorySourceResolver::lastError() const {
  return error;
}
