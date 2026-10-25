/***
 * @file kernel_packer.h
 * @brief Build-time kernel module packing, stub, and budget planning
 */

#ifndef KERNEL_PACKER_H_INCLUDED
#define KERNEL_PACKER_H_INCLUDED

#include <set>
#include <string>
#include <vector>

#include "kernel_module.h"

class KernelModuleLibrary;

enum class KernelRomType { Plain, MegaRom };

struct KernelPackingRequest {
  const KernelModuleLibrary* library;
  std::set<unsigned int> usedDispatch;
  std::set<std::string> requiredModules;
  std::set<std::string> forceOverflowModules;
  std::string dispatcherModuleId;
  unsigned int dispatcherAddress;
  unsigned int tableBaseAddress;
  unsigned int kernelStart;
  unsigned int kernelEnd;
  KernelRomType romType;
  unsigned int overflowStart;
  unsigned int overflowCapacity;
  unsigned int startCodeSize;
  unsigned int userCodeSize;
  unsigned int baseSegment;
  unsigned int maxSegments;
  bool pinGameGraphics;

  KernelPackingRequest();
};

struct KernelModulePlacement {
  std::string moduleId;
  bool resident;
  unsigned int address;
  unsigned int segment;
  unsigned int size;
  KernelPlacementPolicy policy;
};

struct KernelDispatchPlacement {
  unsigned int dispatchIndex;
  std::string exportName;
  std::string moduleId;
  bool paged;
  unsigned int address;
  unsigned int segment;
  unsigned int stubAddress;
};

struct KernelImportPlacement {
  std::string targetExport;
  std::string moduleId;
  bool paged;
  unsigned int address;
  unsigned int segment;
  unsigned int stubAddress;
};

struct KernelPackingResult {
  bool ok;
  std::vector<KernelModulePlacement> placements;
  std::vector<KernelDispatchPlacement> dispatches;
  std::vector<KernelImportPlacement> imports;
  std::vector<std::string> diagnostics;
  std::string error;
  unsigned int tableSize;
  unsigned int tableBaseAddress;
  unsigned int dispatcherAddress;
  unsigned int residentBytes;
  unsigned int overflowBytes;
  std::vector<unsigned char> tableBytes;

  KernelPackingResult();
};

class KernelPacker {
 public:
  static KernelPackingResult pack(const KernelPackingRequest& request);
};

struct KernelCompositionResult {
  bool ok;
  std::vector<unsigned char> image;
  std::string error;

  KernelCompositionResult();
};

class KernelComposer {
 public:
  static KernelCompositionResult compose(const KernelModuleLibrary& library,
                                         const KernelPackingResult& packing,
                                         unsigned int kernelStart,
                                         unsigned int kernelEnd);
};

#endif  // KERNEL_PACKER_H_INCLUDED
