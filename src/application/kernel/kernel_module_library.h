/***
 * @file kernel_module_library.h
 * @brief Read-only kernel module and manifest boundary
 */

#ifndef KERNEL_MODULE_LIBRARY_H_INCLUDED
#define KERNEL_MODULE_LIBRARY_H_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "kernel_module.h"

class KernelModuleLibrary {
 public:
  virtual ~KernelModuleLibrary() {}

  virtual const std::vector<KernelModule>& modules() const = 0;
  virtual const std::vector<KernelFixedRegion>& fixedRegions() const = 0;
  virtual const std::map<unsigned int, std::string>& dispatchExports()
      const = 0;
};

class InMemoryKernelModuleLibrary : public KernelModuleLibrary {
 public:
  std::vector<KernelModule> moduleList;
  std::vector<KernelFixedRegion> regionList;
  std::map<unsigned int, std::string> dispatchMap;

  void addModule(const KernelModule& module) {
    moduleList.push_back(module);
  }

  void addFixedRegion(const KernelFixedRegion& region) {
    regionList.push_back(region);
  }

  void addDispatch(unsigned int index, const std::string& exportName) {
    dispatchMap[index] = exportName;
  }

  const std::vector<KernelModule>& modules() const override {
    return moduleList;
  }

  const std::vector<KernelFixedRegion>& fixedRegions() const override {
    return regionList;
  }

  const std::map<unsigned int, std::string>& dispatchExports() const override {
    return dispatchMap;
  }
};

#endif  // KERNEL_MODULE_LIBRARY_H_INCLUDED
