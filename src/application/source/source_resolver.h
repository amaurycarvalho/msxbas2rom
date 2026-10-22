/***
 * @file source_resolver.h
 * @brief Application source resolver port
 * @author Amaury Carvalho (2026)
 */

#ifndef SOURCE_RESOLVER_H_INCLUDED
#define SOURCE_RESOLVER_H_INCLUDED

#include <memory>
#include <string>

#include "source_document.h"

/***
 * @brief Resolve a logical source name into source text.
 */
class SourceResolver {
 public:
  virtual ~SourceResolver() {}

  virtual bool resolve(const std::string& name, SourceDocument& document) = 0;
  virtual std::string lastError() const = 0;
};

#endif  // SOURCE_RESOLVER_H_INCLUDED
