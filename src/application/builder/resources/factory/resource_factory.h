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

/***
 * @class ResourceFactory
 * @brief Resource factory
 */
class ResourceFactory {
 public:
  static std::unique_ptr<ResourceReader> create(string filename);
  ResourceFactory();
  ~ResourceFactory();
};

#endif  // RESOURCE_FACTORY_H_INCLUDED
