/***
 * @file generated_kernel_module_library.h
 * @brief Generated kernel module library adapter
 */

#ifndef GENERATED_KERNEL_MODULE_LIBRARY_H_INCLUDED
#define GENERATED_KERNEL_MODULE_LIBRARY_H_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "kernel_module.h"
#include "kernel_module_library.h"

class GeneratedKernelModuleLibrary : public KernelModuleLibrary {
 public:
  GeneratedKernelModuleLibrary();

  const std::vector<KernelModule>& modules() const override;
  const std::vector<KernelFixedRegion>& fixedRegions() const override;
  const std::map<unsigned int, std::string>& dispatchExports() const override;

 private:
  std::vector<KernelModule> moduleList;
  std::vector<KernelFixedRegion> regionList;
  std::map<unsigned int, std::string> dispatchMap;
};

#endif  // GENERATED_KERNEL_MODULE_LIBRARY_H_INCLUDED
