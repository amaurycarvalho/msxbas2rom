/***
 * @file resource_factory.h
 * @brief Resource factory header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_FACTORY_H_INCLUDED
#define RESOURCE_FACTORY_H_INCLUDED

#include <memory>

#include "resource_reader.h"

class ResourceContentSource;

/***
 * @class ResourceFactory
 * @brief Resource factory
 */
class ResourceFactory {
 private:
  std::shared_ptr<ResourceContentSource> contentSource;

 public:
  std::shared_ptr<ResourceReader> create(string filename);
  explicit ResourceFactory(std::shared_ptr<ResourceContentSource> contentSource);
  ~ResourceFactory();
};

#endif  // RESOURCE_FACTORY_H_INCLUDED
