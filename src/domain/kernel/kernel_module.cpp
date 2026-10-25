/***
 * @file kernel_module.cpp
 * @brief Kernel module manifest and relocation model implementation
 */

#include "kernel_module.h"

#include <set>

KernelModule::KernelModule()
    : assemblyBase(0),
      policy(KernelPlacementPolicy::FlexHigh),
      fixedAddress(0),
      hasFixedAddress(false),
      forceOverflow(false) {}

unsigned int KernelModule::size() const {
  return static_cast<unsigned int>(bytes.size());
}

unsigned int KernelModule::end() const {
  return assemblyBase + size();
}

const KernelExport* KernelModule::findExport(const std::string& name) const {
  for (std::size_t i = 0; i < exports.size(); i++) {
    if (exports[i].name == name) return &exports[i];
  }
  return nullptr;
}

const char* kernelPlacementPolicyName(KernelPlacementPolicy policy) {
  switch (policy) {
    case KernelPlacementPolicy::Fixed:
      return "fixed";
    case KernelPlacementPolicy::FlexHigh:
      return "flex-high";
    case KernelPlacementPolicy::FlexLow:
      return "flex-low";
    case KernelPlacementPolicy::Overflow:
      return "overflow";
  }
  return "unknown";
}

bool parseKernelPlacementPolicy(const std::string& text,
                                KernelPlacementPolicy& policy) {
  if (text == "fixed") {
    policy = KernelPlacementPolicy::Fixed;
  } else if (text == "flex-high") {
    policy = KernelPlacementPolicy::FlexHigh;
  } else if (text == "flex-low") {
    policy = KernelPlacementPolicy::FlexLow;
  } else if (text == "overflow") {
    policy = KernelPlacementPolicy::Overflow;
  } else {
    return false;
  }
  return true;
}

const KernelModule* KernelModuleManifest::findModule(const std::string& id) const {
  for (std::size_t i = 0; i < modules.size(); i++) {
    if (modules[i].id == id) return &modules[i];
  }
  return nullptr;
}

const KernelModule* KernelModuleManifest::findModuleForExport(
    const std::string& name) const {
  for (std::size_t i = 0; i < modules.size(); i++) {
    if (modules[i].findExport(name)) return &modules[i];
  }
  return nullptr;
}

static bool validateDependencyCycles(const KernelModuleManifest& manifest,
                                     std::string& error) {
  enum State { Unvisited, Visiting, Visited };
  std::vector<State> states(manifest.modules.size(), Unvisited);

  for (std::size_t root = 0; root < manifest.modules.size(); root++) {
    if (states[root] == Visited) continue;

    std::vector<std::size_t> stack;
    std::vector<std::size_t> next;
    stack.push_back(root);
    next.push_back(0);

    while (!stack.empty()) {
      std::size_t index = stack.back();
      states[index] = Visiting;

      if (next.back() >= manifest.modules[index].dependencies.size()) {
        states[index] = Visited;
        stack.pop_back();
        next.pop_back();
        continue;
      }

      const std::string& dependency =
          manifest.modules[index].dependencies[next.back()];
      next.back()++;

      const KernelModule* target = manifest.findModule(dependency);
      if (!target) continue;

      std::size_t targetIndex =
          static_cast<std::size_t>(target - &manifest.modules[0]);
      if (states[targetIndex] == Visiting) {
        error = "dependency cycle includes module '" + dependency + "'";
        return false;
      }
      if (states[targetIndex] == Unvisited) {
        stack.push_back(targetIndex);
        next.push_back(0);
      }
    }
  }

  return true;
}

bool KernelModuleManifest::validate(std::string& error) const {
  std::set<std::string> moduleIds;
  std::set<std::string> exportNames;

  for (std::size_t i = 0; i < modules.size(); i++) {
    const KernelModule& module = modules[i];
    const std::string prefix = "module '" + module.id + "'";

    if (module.id.empty()) {
      error = "module id is empty";
      return false;
    }
    if (module.name.empty()) {
      error = prefix + " has an empty name";
      return false;
    }
    if (!moduleIds.insert(module.id).second) {
      error = "duplicate module id '" + module.id + "'";
      return false;
    }
    if (module.bytes.empty()) {
      error = prefix + " has no bytes";
      return false;
    }
    if (module.end() > 0x10000u) {
      error = prefix + " exceeds the 16-bit address space";
      return false;
    }

    if (module.policy == KernelPlacementPolicy::Fixed &&
        !module.hasFixedAddress) {
      error = prefix + " is fixed but has no fixed address";
      return false;
    }
    if (module.hasFixedAddress) {
      if (module.fixedAddress < 0x4000u || module.fixedAddress >= 0x8000u) {
        error = prefix + " fixed address is outside the kernel page";
        return false;
      }
      if (module.fixedAddress + module.size() > 0x8000u) {
        error = prefix + " overflows the kernel page";
        return false;
      }
      if (module.fixedAddress != module.assemblyBase) {
        error = prefix + " fixed address does not match its assembly base";
        return false;
      }
    }

    std::set<std::string> localExports;
    for (std::size_t j = 0; j < module.exports.size(); j++) {
      const KernelExport& entry = module.exports[j];
      if (entry.name.empty()) {
        error = prefix + " has an export with an empty name";
        return false;
      }
      if (!localExports.insert(entry.name).second) {
        error = prefix + " has duplicate export '" + entry.name + "'";
        return false;
      }
      if (!exportNames.insert(entry.name).second) {
        error = "duplicate export '" + entry.name + "'";
        return false;
      }
      if (entry.offset >= module.size()) {
        error = prefix + " export '" + entry.name + "' is out of range";
        return false;
      }
    }

    std::set<unsigned int> importOffsets;
    for (std::size_t j = 0; j < module.imports.size(); j++) {
      const KernelImport& entry = module.imports[j];
      if (entry.target.empty()) {
        error = prefix + " has an import with an empty target";
        return false;
      }
      if (entry.offset + 2 > module.size()) {
        error = prefix + " import offset is out of range";
        return false;
      }
      if (!importOffsets.insert(entry.offset).second) {
        error = prefix + " has a duplicate import offset";
        return false;
      }
      if (localExports.count(entry.target)) {
        error = prefix + " imports its own export '" + entry.target + "'";
        return false;
      }
    }

    std::set<unsigned int> relocationOffsets;
    for (std::size_t j = 0; j < module.relocations.size(); j++) {
      unsigned int offset = module.relocations[j].offset;
      if (offset + 2 > module.size()) {
        error = prefix + " relocation offset is out of range";
        return false;
      }
      if (!relocationOffsets.insert(offset).second) {
        error = prefix + " has a duplicate relocation offset";
        return false;
      }
    }

    std::set<unsigned int> patchPoints;
    for (std::size_t j = 0; j < module.patchPoints.size(); j++) {
      unsigned int offset = module.patchPoints[j];
      if (offset >= module.size()) {
        error = prefix + " patch point is out of range";
        return false;
      }
      if (!patchPoints.insert(offset).second) {
        error = prefix + " has a duplicate patch point";
        return false;
      }
    }
  }

  for (std::size_t i = 0; i < modules.size(); i++) {
    const KernelModule& module = modules[i];
    std::set<std::string> localDependencies;
    for (std::size_t j = 0; j < module.dependencies.size(); j++) {
      const std::string& dependency = module.dependencies[j];
      if (dependency == module.id) {
        error = "module '" + module.id + "' depends on itself";
        return false;
      }
      if (!findModule(dependency)) {
        error = "module '" + module.id + "' depends on missing module '" +
                dependency + "'";
        return false;
      }
      if (!localDependencies.insert(dependency).second) {
        error = "module '" + module.id + "' has a duplicate dependency";
        return false;
      }
    }
  }

  for (std::size_t i = 0; i < modules.size(); i++) {
    const KernelModule& module = modules[i];
    for (std::size_t j = 0; j < module.imports.size(); j++) {
      const std::string& target = module.imports[j].target;
      if (!exportNames.count(target)) {
        error = "module '" + module.id + "' imports missing export '" +
                target + "'";
        return false;
      }
    }
  }

  for (std::size_t i = 0; i < fixedRegions.size(); i++) {
    const KernelFixedRegion& region = fixedRegions[i];
    if (region.id.empty()) {
      error = "fixed region id is empty";
      return false;
    }
    if (region.start >= region.end) {
      error = "fixed region '" + region.id + "' has an invalid range";
      return false;
    }
    if (region.end > 0x8000u) {
      error = "fixed region '" + region.id + "' exceeds the kernel page";
      return false;
    }
    for (std::size_t j = i + 1; j < fixedRegions.size(); j++) {
      const KernelFixedRegion& other = fixedRegions[j];
      if (region.start < other.end && other.start < region.end) {
        error = "fixed regions '" + region.id + "' and '" + other.id +
                "' overlap";
        return false;
      }
    }
  }

  return true;
}

bool KernelModuleManifest::validateAcyclic(std::string& error) const {
  return validateDependencyCycles(*this, error);
}

bool diffRelocations(const std::vector<unsigned char>& baseA,
                     const std::vector<unsigned char>& baseB, int baseDelta,
                     KernelRelocationDiff& diff, std::string& error) {
  diff.offsets.clear();
  diff.values.clear();

  if (baseA.size() != baseB.size()) {
    error = "dual-base assemblies have different sizes";
    return false;
  }

  unsigned int expected = static_cast<unsigned int>(baseDelta) & 0xFFFFu;

  for (std::size_t offset = 0; offset + 1 < baseA.size(); offset++) {
    unsigned int valueA = baseA[offset] | (baseA[offset + 1] << 8);
    unsigned int valueB = baseB[offset] | (baseB[offset + 1] << 8);
    if (valueA == valueB) continue;
    if (((valueB - valueA) & 0xFFFFu) != expected) continue;
    diff.offsets.push_back(static_cast<unsigned int>(offset));
    diff.values.push_back(valueB);
  }

  if (diff.offsets.empty() && expected != 0) {
    error = "dual-base assemblies contain no relocatable references";
    return false;
  }

  return true;
}

bool applyModuleImports(
    std::vector<unsigned char>& bytes,
    const std::vector<KernelImport>& imports,
    const std::map<std::string, unsigned int>& stubAddresses,
    std::string& error) {
  for (std::size_t i = 0; i < imports.size(); i++) {
    const KernelImport& entry = imports[i];
    std::map<std::string, unsigned int>::const_iterator stub =
        stubAddresses.find(entry.target);
    if (stub == stubAddresses.end()) {
      error = "no stub address for import '" + entry.target + "'";
      return false;
    }
    if (entry.offset + 2 > bytes.size()) {
      error = "import offset is out of range";
      return false;
    }
    bytes[entry.offset] = static_cast<unsigned char>(stub->second & 0xFF);
    bytes[entry.offset + 1] =
        static_cast<unsigned char>((stub->second >> 8) & 0xFF);
  }
  return true;
}

bool applyRelocations(std::vector<unsigned char>& bytes,
                      unsigned int assemblyBase, unsigned int finalBase,
                      const std::vector<KernelRelocation>& relocations,
                      std::string& error) {
  int delta = static_cast<int>(finalBase) - static_cast<int>(assemblyBase);

  for (std::size_t i = 0; i < relocations.size(); i++) {
    unsigned int offset = relocations[i].offset;
    if (offset + 2 > bytes.size()) {
      error = "relocation offset is out of range";
      return false;
    }

    unsigned int value = bytes[offset] | (bytes[offset + 1] << 8);
    int adjusted = static_cast<int>(value) + delta;
    if (adjusted < 0 || adjusted > 0xFFFF) {
      error = "relocation target is outside the 16-bit address space";
      return false;
    }

    bytes[offset] = static_cast<unsigned char>(adjusted & 0xFF);
    bytes[offset + 1] = static_cast<unsigned char>((adjusted >> 8) & 0xFF);
  }

  return true;
}
