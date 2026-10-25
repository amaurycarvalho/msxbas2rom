/***
 * @file kernel_size_map.cpp
 * @brief Kernel symbol ownership and size map implementation
 */

#include "kernel_size_map.h"

#include <algorithm>
#include <map>
#include <set>

unsigned int KernelModuleExtent::size() const { return end - start; }

unsigned int KernelRoutineExtent::size() const { return end - start; }

namespace {

struct AddressOwner {
  unsigned int address;
  std::string owner;
  std::string name;
};

bool byAddress(const AddressOwner& left, const AddressOwner& right) {
  if (left.address != right.address) return left.address < right.address;
  return left.name < right.name;
}

bool byExtentStart(const KernelModuleExtent& left,
                   const KernelModuleExtent& right) {
  return left.start < right.start;
}

bool byRoutineAddress(const KernelRoutineExtent& left,
                      const KernelRoutineExtent& right) {
  if (left.start != right.start) return left.start < right.start;
  return left.name < right.name;
}

}  // namespace

std::vector<KernelModuleExtent> KernelSizeMap::moduleExtents(
    const std::vector<KernelSymbol>& symbols, unsigned int kernelStart,
    unsigned int kernelEnd) {
  std::vector<AddressOwner> entries;

  for (std::size_t i = 0; i < symbols.size(); i++) {
    const KernelSymbol& symbol = symbols[i];
    if (symbol.owner.empty()) continue;
    if (symbol.address < kernelStart || symbol.address >= kernelEnd) continue;
    AddressOwner entry;
    entry.address = symbol.address;
    entry.owner = symbol.owner;
    entry.name = symbol.name;
    entries.push_back(entry);
  }

  std::sort(entries.begin(), entries.end(), byAddress);

  std::vector<KernelModuleExtent> extents;
  std::size_t index = 0;
  while (index < entries.size()) {
    unsigned int address = entries[index].address;
    const std::string& owner = entries[index].owner;

    std::size_t next = index;
    while (next < entries.size() && entries[next].address == address) next++;

    unsigned int end = (next < entries.size()) ? entries[next].address
                                               : kernelEnd;

    if (!extents.empty() && extents.back().owner == owner &&
        extents.back().end == address) {
      extents.back().end = end;
    } else {
      KernelModuleExtent extent;
      extent.owner = owner;
      extent.start = address;
      extent.end = end;
      extents.push_back(extent);
    }

    index = next;
  }

  std::sort(extents.begin(), extents.end(), byExtentStart);
  return extents;
}

std::vector<KernelRoutineExtent> KernelSizeMap::routineExtents(
    const std::vector<KernelSymbol>& symbols,
    const std::vector<KernelModuleExtent>& modules) {
  std::map<std::string, std::vector<const KernelSymbol*> > grouped;

  for (std::size_t i = 0; i < symbols.size(); i++) {
    const KernelSymbol& symbol = symbols[i];
    if (symbol.owner.empty()) continue;
    grouped[symbol.owner].push_back(&symbol);
  }

  std::vector<KernelRoutineExtent> routines;

  for (std::map<std::string, std::vector<const KernelSymbol*> >::iterator it =
           grouped.begin();
       it != grouped.end(); ++it) {
    std::vector<const KernelSymbol*>& entries = it->second;

    std::sort(entries.begin(), entries.end(),
              [](const KernelSymbol* left, const KernelSymbol* right) {
                if (left->address != right->address)
                  return left->address < right->address;
                return left->name < right->name;
              });

    for (std::size_t i = 0; i < entries.size(); i++) {
      const KernelSymbol& symbol = *entries[i];

      unsigned int end = 0;
      if (i + 1 < entries.size() &&
          entries[i + 1]->address > symbol.address) {
        end = entries[i + 1]->address;
      } else {
        for (std::size_t j = 0; j < modules.size(); j++) {
          if (modules[j].owner == symbol.owner &&
              symbol.address >= modules[j].start &&
              symbol.address < modules[j].end) {
            end = modules[j].end;
            break;
          }
        }
      }

      if (end <= symbol.address) continue;

      KernelRoutineExtent routine;
      routine.name = symbol.name;
      routine.owner = symbol.owner;
      routine.start = symbol.address;
      routine.end = end;
      routines.push_back(routine);
    }
  }

  std::sort(routines.begin(), routines.end(), byRoutineAddress);
  return routines;
}
