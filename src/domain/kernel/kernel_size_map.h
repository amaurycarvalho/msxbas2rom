/***
 * @file kernel_size_map.h
 * @brief Kernel symbol ownership and size map
 */

#ifndef KERNEL_SIZE_MAP_H_INCLUDED
#define KERNEL_SIZE_MAP_H_INCLUDED

#include <string>
#include <vector>

struct KernelSymbol {
  std::string name;
  unsigned int address;
  std::string owner;
};

struct KernelModuleExtent {
  std::string owner;
  unsigned int start;
  unsigned int end;

  unsigned int size() const;
};

struct KernelRoutineExtent {
  std::string name;
  std::string owner;
  unsigned int start;
  unsigned int end;

  unsigned int size() const;
};

class KernelSizeMap {
 public:
  static std::vector<KernelModuleExtent> moduleExtents(
      const std::vector<KernelSymbol>& symbols, unsigned int kernelStart,
      unsigned int kernelEnd);

  static std::vector<KernelRoutineExtent> routineExtents(
      const std::vector<KernelSymbol>& symbols,
      const std::vector<KernelModuleExtent>& modules);
};

#endif  // KERNEL_SIZE_MAP_H_INCLUDED
