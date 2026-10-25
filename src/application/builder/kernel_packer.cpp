/***
 * @file kernel_packer.cpp
 * @brief Build-time kernel module packing, stub, and budget planning
 */

#include "kernel_packer.h"

#include <algorithm>
#include <map>

#include "kernel_central_table.h"
#include "kernel_module_library.h"

namespace {

struct Interval {
  unsigned int start;
  unsigned int end;
};

struct Hole {
  unsigned int start;
  unsigned int end;

  unsigned int size() const { return end - start; }
};

struct ModuleState {
  const KernelModule* module;
  bool overflow;
  unsigned int address;
  unsigned int segment;
};

bool intervalLess(const Interval& left, const Interval& right) {
  if (left.start != right.start) return left.start < right.start;
  return left.end < right.end;
}

void addInterval(std::vector<Interval>& intervals, unsigned int start,
                 unsigned int end) {
  if (start >= end) return;
  Interval interval;
  interval.start = start;
  interval.end = end;
  intervals.push_back(interval);
}

std::vector<Interval> mergeIntervals(std::vector<Interval> intervals) {
  std::sort(intervals.begin(), intervals.end(), intervalLess);
  std::vector<Interval> merged;
  for (std::size_t i = 0; i < intervals.size(); i++) {
    if (merged.empty() || intervals[i].start > merged.back().end) {
      merged.push_back(intervals[i]);
    } else if (intervals[i].end > merged.back().end) {
      merged.back().end = intervals[i].end;
    }
  }
  return merged;
}

std::vector<Hole> findHoles(const std::vector<Interval>& intervals,
                            unsigned int kernelStart, unsigned int kernelEnd) {
  std::vector<Hole> holes;
  unsigned int cursor = kernelStart;
  for (std::size_t i = 0; i < intervals.size(); i++) {
    if (intervals[i].start > cursor) {
      Hole hole;
      hole.start = cursor;
      hole.end = intervals[i].start;
      holes.push_back(hole);
    }
    if (intervals[i].end > cursor) cursor = intervals[i].end;
  }
  if (cursor < kernelEnd) {
    Hole hole;
    hole.start = cursor;
    hole.end = kernelEnd;
    holes.push_back(hole);
  }
  return holes;
}

bool isGameGraphics(const KernelModule& module) {
  const std::string& id = module.id;
  const std::string& name = module.name;
  const std::string& owner = module.owner;
  return module.group == "game-graphics" ||
         id.find("game-graphics") != std::string::npos ||
         name.find("game-graphics") != std::string::npos ||
         owner == "31_cmd" || owner == "90-graphics";
}

bool policyOrder(const KernelModule* left, const KernelModule* right) {
  if (left->policy != right->policy) {
    return static_cast<int>(left->policy) < static_cast<int>(right->policy);
  }
  if (left->size() != right->size()) return left->size() > right->size();
  return left->id < right->id;
}

}  // namespace

KernelPackingRequest::KernelPackingRequest()
    : library(nullptr),
      dispatcherModuleId("dispatcher"),
      dispatcherAddress(0x4000),
      tableBaseAddress(0x4000),
      kernelStart(0x4000),
      kernelEnd(0x8000),
      romType(KernelRomType::Plain),
      overflowStart(0x8000),
      overflowCapacity(0x4000),
      startCodeSize(0),
      userCodeSize(0),
      baseSegment(0),
      maxSegments(256),
      pinGameGraphics(false) {}

KernelPackingResult::KernelPackingResult()
    : ok(false),
      tableSize(0),
      tableBaseAddress(0),
      dispatcherAddress(0),
      residentBytes(0),
      overflowBytes(0) {}

KernelPackingResult KernelPacker::pack(const KernelPackingRequest& request) {
  KernelPackingResult result;

  if (!request.library) {
    result.error = "kernel module library is missing";
    return result;
  }

  const std::vector<KernelModule>& modules = request.library->modules();
  if (modules.empty()) {
    result.error = "kernel module library is empty";
    return result;
  }

  std::map<std::string, const KernelModule*> byId;
  std::map<std::string, const KernelModule*> byExport;
  for (std::size_t i = 0; i < modules.size(); i++) {
    const KernelModule& module = modules[i];
    if (module.id.empty() || module.size() == 0) {
      result.error = "kernel module has an empty id or no bytes";
      return result;
    }
    if (!byId.insert(std::make_pair(module.id, &module)).second) {
      result.error = "duplicate kernel module id '" + module.id + "'";
      return result;
    }
    for (std::size_t j = 0; j < module.exports.size(); j++) {
      const std::string& name = module.exports[j].name;
      if (!byExport.insert(std::make_pair(name, &module)).second) {
        result.error = "duplicate kernel export '" + name + "'";
        return result;
      }
    }
  }

  std::set<std::string> usedIds;
  for (std::size_t i = 0; i < modules.size(); i++) {
    if (modules[i].policy == KernelPlacementPolicy::Fixed) {
      usedIds.insert(modules[i].id);
    }
  }
  for (std::set<std::string>::const_iterator it =
           request.requiredModules.begin();
       it != request.requiredModules.end(); ++it) {
    if (!byId.count(*it)) {
      result.error = "required module '" + *it + "' is missing";
      return result;
    }
    usedIds.insert(*it);
  }

  const std::map<unsigned int, std::string>& dispatchMap =
      request.library->dispatchExports();

  for (std::set<unsigned int>::const_iterator it =
           request.usedDispatch.begin();
       it != request.usedDispatch.end(); ++it) {
    std::map<unsigned int, std::string>::const_iterator entry =
        dispatchMap.find(*it);
    if (entry == dispatchMap.end()) {
      result.error = "missing export name for dispatch index " +
                     std::to_string(*it);
      return result;
    }
    std::map<std::string, const KernelModule*>::const_iterator module =
        byExport.find(entry->second);
    if (module == byExport.end()) {
      result.error = "no module exports dispatch name '" + entry->second +
                     "'";
      return result;
    }
    usedIds.insert(module->second->id);
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (std::set<std::string>::const_iterator it = usedIds.begin();
         it != usedIds.end(); ++it) {
      std::map<std::string, const KernelModule*>::const_iterator module =
          byId.find(*it);
      if (module == byId.end()) {
        result.error = "missing kernel module '" + *it + "'";
        return result;
      }
      for (std::size_t i = 0; i < module->second->dependencies.size(); i++) {
        const std::string& dependency = module->second->dependencies[i];
        if (!byId.count(dependency)) {
          result.error = "module '" + *it + "' depends on missing module '" +
                         dependency + "'";
          return result;
        }
        if (usedIds.insert(dependency).second) changed = true;
      }
      for (std::size_t i = 0; i < module->second->imports.size(); i++) {
        const std::string& target = module->second->imports[i].target;
        std::map<std::string, const KernelModule*>::const_iterator imported =
            byExport.find(target);
        if (imported == byExport.end()) {
          result.error = "module '" + *it + "' imports missing export '" +
                         target + "'";
          return result;
        }
        if (usedIds.insert(imported->second->id).second) changed = true;
      }
    }
  }

  std::set<std::string> usedDispatchExports;
  for (std::set<unsigned int>::const_iterator it =
           request.usedDispatch.begin();
       it != request.usedDispatch.end(); ++it) {
    std::map<unsigned int, std::string>::const_iterator entry =
        dispatchMap.find(*it);
    if (entry == dispatchMap.end()) {
      result.error = "missing export name for dispatch index " +
                     std::to_string(*it);
      return result;
    }
    usedDispatchExports.insert(entry->second);
  }

  std::set<std::string> usedImports;
  for (std::set<std::string>::const_iterator it = usedIds.begin();
       it != usedIds.end(); ++it) {
    const KernelModule* module = byId[*it];
    for (std::size_t i = 0; i < module->imports.size(); i++) {
      const std::string& target = module->imports[i].target;
      if (!usedDispatchExports.count(target)) {
        usedImports.insert(target);
      }
    }
  }

  std::vector<Interval> fixedIntervals;
  std::vector<Interval> allIntervals;
  std::map<std::string, ModuleState> states;
  std::vector<KernelModulePlacement> fixedPlacements;

  for (std::set<std::string>::const_iterator it = usedIds.begin();
       it != usedIds.end(); ++it) {
    const KernelModule* module = byId[*it];
    ModuleState state;
    state.module = module;
    state.overflow = false;
    state.address = 0;
    state.segment = 0;

    if (module->policy == KernelPlacementPolicy::Fixed &&
        !module->hasFixedAddress) {
      result.error = "fixed module '" + module->id +
                     "' has no fixed address";
      return result;
    }

    if (module->hasFixedAddress) {
      if (module->fixedAddress < request.kernelStart ||
          module->fixedAddress + module->size() > request.kernelEnd) {
        result.error = "module '" + module->id +
                       "' is outside the kernel page";
        return result;
      }
      state.address = module->fixedAddress;
      state.segment = 0;
      addInterval(fixedIntervals, module->fixedAddress,
                  module->fixedAddress + module->size());
      addInterval(allIntervals, module->fixedAddress,
                  module->fixedAddress + module->size());

      KernelModulePlacement placement;
      placement.moduleId = module->id;
      placement.resident = true;
      placement.address = module->fixedAddress;
      placement.segment = 0;
      placement.size = module->size();
      placement.policy = module->policy;
      fixedPlacements.push_back(placement);
    } else if (module->policy == KernelPlacementPolicy::Overflow) {
      state.overflow = true;
    }

    states[module->id] = state;
  }

  std::vector<Interval> fixedRegions;
  const std::vector<KernelFixedRegion>& regions =
      request.library->fixedRegions();
  for (std::size_t i = 0; i < regions.size(); i++) {
    const KernelFixedRegion& region = regions[i];
    if (region.start < request.kernelStart || region.end > request.kernelEnd ||
        region.start >= region.end) {
      result.error = "fixed region '" + region.id +
                     "' is outside the kernel page";
      return result;
    }
    addInterval(fixedRegions, region.start, region.end);
    addInterval(allIntervals, region.start, region.end);
  }

  std::vector<Interval> mergedFixedModules =
      mergeIntervals(fixedIntervals);
  for (std::size_t i = 1; i < mergedFixedModules.size(); i++) {
    if (mergedFixedModules[i].start < mergedFixedModules[i - 1].end) {
      result.error = "fixed kernel modules overlap";
      return result;
    }
  }

  if (request.dispatcherAddress != 0 &&
      (request.dispatcherAddress < request.kernelStart ||
       request.dispatcherAddress >= request.kernelEnd)) {
    result.error = "dispatcher address is outside the kernel page";
    return result;
  }

  unsigned int tableBase = request.tableBaseAddress;
  if (tableBase == 0) {
    unsigned int initialTableSize =
        static_cast<unsigned int>(request.usedDispatch.size()) * 3u;
    std::vector<Interval> merged = mergeIntervals(allIntervals);
    std::vector<Hole> holes =
        findHoles(merged, request.kernelStart, request.kernelEnd);
    bool found = false;
    for (std::size_t i = 0; i < holes.size(); i++) {
      if (holes[i].size() >= initialTableSize) {
        tableBase = holes[i].start;
        found = true;
        break;
      }
    }
    if (!found) {
      result.error = "no kernel hole fits the central table";
      return result;
    }
  } else if (tableBase < request.kernelStart ||
             tableBase >= request.kernelEnd) {
    result.error = "central table base is outside the kernel page";
    return result;
  }

  std::set<std::string> forcedOverflow;
  for (std::set<std::string>::const_iterator it = usedIds.begin();
       it != usedIds.end(); ++it) {
    const KernelModule* module = byId[*it];
    if (module->policy == KernelPlacementPolicy::Overflow ||
        module->forceOverflow ||
        request.forceOverflowModules.count(module->id)) {
      forcedOverflow.insert(module->id);
    }
  }
  for (std::set<std::string>::const_iterator it =
           request.forceOverflowModules.begin();
       it != request.forceOverflowModules.end(); ++it) {
    if (!byId.count(*it)) {
      result.error = "forced overflow module '" + *it + "' is missing";
      return result;
    }
    if (usedIds.count(*it)) forcedOverflow.insert(*it);
  }
  for (std::set<std::string>::const_iterator it = forcedOverflow.begin();
       it != forcedOverflow.end(); ++it) {
    const KernelModule* module = byId[*it];
    if (!module->hasFixedAddress) continue;
    result.error = "module '" + module->id +
                   "' is fixed and cannot be relocated; convert inbound "
                   "references to import fixups first";
    return result;
  }

  std::vector<KernelModulePlacement> residentPlacements;
  unsigned int tableSize = 0;

  for (;;) {
    tableSize = 0;
    for (std::set<unsigned int>::const_iterator it =
             request.usedDispatch.begin();
         it != request.usedDispatch.end(); ++it) {
      const KernelModule* module = byExport[dispatchMap.at(*it)];
      bool paged = forcedOverflow.count(module->id) != 0 &&
                   request.romType == KernelRomType::MegaRom;
      tableSize += paged ? 6u : 3u;
    }
    for (std::set<std::string>::const_iterator it = usedImports.begin();
         it != usedImports.end(); ++it) {
      std::map<std::string, const KernelModule*>::const_iterator module =
          byExport.find(*it);
      if (module == byExport.end()) {
        result.error = "no module exports imported name '" + *it + "'";
        return result;
      }
      bool paged = forcedOverflow.count(module->second->id) != 0 &&
                   request.romType == KernelRomType::MegaRom;
      tableSize += paged ? 6u : 3u;
    }

    if (tableSize > 0 && tableBase + tableSize > request.kernelEnd) {
      result.error = "central table exceeds the kernel page";
      return result;
    }

    std::vector<Interval> occupied = allIntervals;
    if (tableSize > 0) {
      addInterval(occupied, tableBase, tableBase + tableSize);
    }
    std::vector<Interval> merged = mergeIntervals(occupied);
    std::vector<Hole> holes =
        findHoles(merged, request.kernelStart, request.kernelEnd);

    std::vector<const KernelModule*> flex;
    for (std::set<std::string>::const_iterator it = usedIds.begin();
         it != usedIds.end(); ++it) {
      const KernelModule* module = byId[*it];
      if (module->policy == KernelPlacementPolicy::Fixed) continue;
      if (module->policy == KernelPlacementPolicy::Overflow) continue;
      if (module->hasFixedAddress) continue;
      if (forcedOverflow.count(module->id)) continue;
      flex.push_back(module);
    }
    std::sort(flex.begin(), flex.end(), policyOrder);

    residentPlacements.clear();
    std::set<std::string> handledAtomicGroups;
    for (std::size_t i = 0; i < flex.size(); i++) {
      const KernelModule* module = flex[i];
      if (isGameGraphics(*module)) {
        const std::string groupKey =
            module->group.empty() ? "game-graphics" : module->group;
        if (handledAtomicGroups.count(groupKey)) continue;

        std::vector<const KernelModule*> group;
        for (std::size_t j = 0; j < flex.size(); j++) {
          const KernelModule* candidate = flex[j];
          if (!isGameGraphics(*candidate)) continue;
          const std::string candidateGroup =
              candidate->group.empty() ? "game-graphics" : candidate->group;
          if (candidateGroup == groupKey) group.push_back(candidate);
        }

        bool anyForced = false;
        for (std::size_t j = 0; j < group.size(); j++) {
          if (forcedOverflow.count(group[j]->id)) anyForced = true;
        }
        if (anyForced) {
          for (std::size_t j = 0; j < group.size(); j++) {
            forcedOverflow.insert(group[j]->id);
          }
          handledAtomicGroups.insert(groupKey);
          continue;
        }

        std::vector<Hole> trialHoles = holes;
        std::vector<KernelModulePlacement> groupPlacements;
        bool groupFits = true;
        for (std::size_t j = 0; j < group.size(); j++) {
          const KernelModule* candidate = group[j];
          int best = -1;
          for (std::size_t h = 0; h < trialHoles.size(); h++) {
            if (trialHoles[h].size() < candidate->size()) continue;
            if (best < 0 || trialHoles[h].size() < trialHoles[best].size()) {
              best = (int)h;
            }
          }
          if (best < 0) {
            groupFits = false;
            break;
          }
          KernelModulePlacement placement;
          placement.moduleId = candidate->id;
          placement.resident = true;
          placement.address = trialHoles[best].start;
          placement.segment = 0;
          placement.size = candidate->size();
          placement.policy = candidate->policy;
          groupPlacements.push_back(placement);
          trialHoles[best].start += candidate->size();
          if (trialHoles[best].start >= trialHoles[best].end) {
            trialHoles.erase(trialHoles.begin() + best);
          }
        }

        if (!groupFits) {
          if (request.pinGameGraphics) {
            result.error =
                "game graphics group is pinned but does not fit the kernel "
                "page";
            return result;
          }
          for (std::size_t j = 0; j < group.size(); j++) {
            forcedOverflow.insert(group[j]->id);
          }
          handledAtomicGroups.insert(groupKey);
          continue;
        }

        holes = trialHoles;
        for (std::size_t j = 0; j < groupPlacements.size(); j++) {
          residentPlacements.push_back(groupPlacements[j]);
        }
        handledAtomicGroups.insert(groupKey);
        continue;
      }

      int best = -1;
      for (std::size_t h = 0; h < holes.size(); h++) {
        if (holes[h].size() < module->size()) continue;
        if (best < 0 || holes[h].size() < holes[best].size()) best = (int)h;
      }

      if (best < 0) {
        forcedOverflow.insert(module->id);
        continue;
      }

      KernelModulePlacement placement;
      placement.moduleId = module->id;
      placement.resident = true;
      placement.address = holes[best].start;
      placement.segment = 0;
      placement.size = module->size();
      placement.policy = module->policy;
      residentPlacements.push_back(placement);

      holes[best].start += module->size();
      if (holes[best].start >= holes[best].end) {
        holes.erase(holes.begin() + best);
      }
    }

    bool added = false;
    for (std::set<std::string>::const_iterator it = forcedOverflow.begin();
         it != forcedOverflow.end(); ++it) {
      if (states[*it].overflow) continue;
      states[*it].overflow = true;
      added = true;
    }
    if (!added) break;
  }

  std::map<std::string, KernelModulePlacement> placementById;
  for (std::size_t i = 0; i < fixedPlacements.size(); i++) {
    placementById[fixedPlacements[i].moduleId] = fixedPlacements[i];
  }
  for (std::size_t i = 0; i < residentPlacements.size(); i++) {
    placementById[residentPlacements[i].moduleId] = residentPlacements[i];
  }

  std::vector<const KernelModule*> overflowModules;
  for (std::set<std::string>::const_iterator it = usedIds.begin();
       it != usedIds.end(); ++it) {
    const KernelModule* module = byId[*it];
    if (forcedOverflow.count(module->id)) {
      overflowModules.push_back(module);
    }
  }
  std::sort(overflowModules.begin(), overflowModules.end(), policyOrder);

  unsigned int overflowOffset = 0;
  unsigned int overflowBytes = 0;
  for (std::size_t i = 0; i < overflowModules.size(); i++) {
    const KernelModule* module = overflowModules[i];
    unsigned int address = 0;
    unsigned int segment = 0;

    if (request.romType == KernelRomType::MegaRom) {
      if (module->size() > 0x2000) {
        result.error = "module '" + module->id +
                       "' is too large for a MegaROM 8 KB segment";
        return result;
      }
      overflowOffset = (overflowOffset + 0x1FFF) & ~0x1FFFu;
      segment = request.baseSegment + overflowOffset / 0x2000;
      if (segment >= request.baseSegment + request.maxSegments) {
        result.error =
            "MegaROM overflow exceeds the mapper limit: segment " +
            std::to_string(segment) + " >= " +
            std::to_string(request.baseSegment + request.maxSegments);
        return result;
      }
      address = request.overflowStart + (overflowOffset % 0x2000);
      overflowOffset += module->size();
    } else {
      address = request.overflowStart + overflowOffset;
      segment = 0;
      overflowOffset += module->size();
    }

    if (overflowOffset > request.overflowCapacity) {
      result.error = "module '" + module->id + "' requires " +
                     std::to_string(module->size()) + " bytes; " +
                     std::to_string(request.overflowCapacity -
                                    (overflowOffset - module->size())) +
                     " available in overflow";
      return result;
    }

    KernelModulePlacement placement;
    placement.moduleId = module->id;
    placement.resident = false;
    placement.address = address;
    placement.segment = segment;
    placement.size = module->size();
    placement.policy = module->policy;
    placementById[module->id] = placement;

    if (isGameGraphics(*module)) {
      result.diagnostics.push_back(
          "game graphics module '" + module->id + "' placed in overflow");
    }
  }
  overflowBytes = overflowOffset;

  if (request.romType == KernelRomType::Plain) {
    unsigned int total = request.startCodeSize + request.userCodeSize +
                         overflowBytes;
    if (total > 0x4000) {
      result.error = "Plain ROM code budget exceeded (plain ROM limit): start code " +
                     std::to_string(request.startCodeSize) + " + user code " +
                     std::to_string(request.userCodeSize) + " + overflow " +
                     std::to_string(overflowBytes) + " = " +
                     std::to_string(total) +
                     " > 16384; use MegaROM";
      return result;
    }
  }

  for (std::map<std::string, KernelModulePlacement>::const_iterator it =
           placementById.begin();
       it != placementById.end(); ++it) {
    result.placements.push_back(it->second);
    if (it->second.resident) {
      result.residentBytes += it->second.size;
    } else {
      result.diagnostics.push_back(
          "module '" + it->first + "' does not fit any kernel hole (size " +
          std::to_string(it->second.size) + "); placed in overflow");
    }
  }
  result.residentBytes += tableSize;

  std::vector<Interval> residentIntervals = allIntervals;
  for (std::size_t i = 0; i < residentPlacements.size(); i++) {
    addInterval(residentIntervals, residentPlacements[i].address,
                residentPlacements[i].address + residentPlacements[i].size);
  }
  if (tableSize > 0) {
    addInterval(residentIntervals, tableBase, tableBase + tableSize);
  }
  std::vector<Interval> mergedResident = mergeIntervals(residentIntervals);
  unsigned int residentFootprint = 0;
  for (std::size_t i = 0; i < mergedResident.size(); i++) {
    residentFootprint += mergedResident[i].end - mergedResident[i].start;
  }
  if (residentFootprint > 0x4000) {
    result.diagnostics.push_back(
        "resident kernel size " + std::to_string(residentFootprint) +
        " exceeds 16384 bytes by " +
        std::to_string(residentFootprint - 0x4000));
  } else {
    result.diagnostics.push_back(
        "resident kernel uses " + std::to_string(residentFootprint) +
        " of 16384 bytes");
  }

  unsigned int dispatcherAddress = request.dispatcherAddress;
  if (dispatcherAddress == 0) {
    std::map<std::string, KernelModulePlacement>::const_iterator dispatcher =
        placementById.find(request.dispatcherModuleId);
    if (dispatcher != placementById.end() && dispatcher->second.resident) {
      dispatcherAddress = dispatcher->second.address;
    }
  }

  unsigned int cursor = tableBase;
  for (std::set<unsigned int>::const_iterator it =
           request.usedDispatch.begin();
       it != request.usedDispatch.end(); ++it) {
    const std::string& exportName = dispatchMap.at(*it);
    const KernelModule* module = byExport[exportName];
    const KernelExport* entry = module->findExport(exportName);
    if (!entry) {
      result.error = "module '" + module->id + "' does not export '" +
                     exportName + "'";
      return result;
    }
    std::map<std::string, KernelModulePlacement>::const_iterator placement =
        placementById.find(module->id);
    if (placement == placementById.end()) {
      result.error = "module '" + module->id + "' was not placed";
      return result;
    }

    KernelDispatchPlacement dispatch;
    dispatch.dispatchIndex = *it;
    dispatch.exportName = exportName;
    dispatch.moduleId = module->id;
    dispatch.paged = !placement->second.resident &&
                     request.romType == KernelRomType::MegaRom;
    if (dispatch.paged && dispatcherAddress == 0) {
      result.error =
          "dispatcher must be resident for paged dispatch entries";
      return result;
    }
    dispatch.address = placement->second.address + entry->offset;
    dispatch.segment = placement->second.segment;
    dispatch.stubAddress = cursor;

    cursor += dispatch.paged ? 6u : 3u;
    result.dispatches.push_back(dispatch);
  }

  for (std::set<std::string>::const_iterator it = usedImports.begin();
       it != usedImports.end(); ++it) {
    const std::string& exportName = *it;
    std::map<std::string, const KernelModule*>::const_iterator module =
        byExport.find(exportName);
    if (module == byExport.end()) {
      result.error = "no module exports imported name '" + exportName + "'";
      return result;
    }
    const KernelExport* entry = module->second->findExport(exportName);
    if (!entry) {
      result.error = "module '" + module->second->id +
                     "' does not export import '" + exportName + "'";
      return result;
    }
    std::map<std::string, KernelModulePlacement>::const_iterator placement =
        placementById.find(module->second->id);
    if (placement == placementById.end()) {
      result.error = "module '" + module->second->id + "' was not placed";
      return result;
    }

    KernelImportPlacement imported;
    imported.targetExport = exportName;
    imported.moduleId = module->second->id;
    imported.paged = !placement->second.resident &&
                     request.romType == KernelRomType::MegaRom;
    if (imported.paged && dispatcherAddress == 0) {
      result.error =
          "dispatcher must be resident for paged import entries";
      return result;
    }
    imported.address = placement->second.address + entry->offset;
    imported.segment = placement->second.segment;
    imported.stubAddress = cursor;

    cursor += imported.paged ? 6u : 3u;
    result.imports.push_back(imported);
  }

  if (cursor != tableBase + tableSize) {
    result.error = "central table size does not match the generated stubs";
    return result;
  }

  for (std::size_t i = 0; i < result.dispatches.size(); i++) {
    const KernelDispatchPlacement& dispatch = result.dispatches[i];
    std::vector<unsigned char> entry;
    if (dispatch.paged) {
      entry = KernelCentralTable::pagedStub(
          dispatcherAddress, dispatch.segment, dispatch.address);
    } else {
      entry = KernelCentralTable::residentStub(dispatch.address);
    }
    result.tableBytes.insert(result.tableBytes.end(), entry.begin(),
                             entry.end());
  }
  for (std::size_t i = 0; i < result.imports.size(); i++) {
    const KernelImportPlacement& imported = result.imports[i];
    std::vector<unsigned char> entry;
    if (imported.paged) {
      entry = KernelCentralTable::pagedStub(
          dispatcherAddress, imported.segment, imported.address);
    } else {
      entry = KernelCentralTable::residentStub(imported.address);
    }
    result.tableBytes.insert(result.tableBytes.end(), entry.begin(),
                             entry.end());
  }

  result.tableSize = tableSize;
  result.tableBaseAddress = tableBase;
  result.dispatcherAddress = dispatcherAddress;
  result.overflowBytes = overflowBytes;
  result.ok = true;
  return result;
}

KernelCompositionResult::KernelCompositionResult() : ok(false) {}

KernelCompositionResult KernelComposer::compose(
    const KernelModuleLibrary& library,
    const KernelPackingResult& packing, unsigned int kernelStart,
    unsigned int kernelEnd) {
  KernelCompositionResult result;

  if (!packing.ok) {
    result.error = "kernel packing failed";
    return result;
  }
  if (kernelEnd <= kernelStart) {
    result.error = "invalid kernel page range";
    return result;
  }

  result.image.assign(kernelEnd - kernelStart, 0);

  if (!packing.tableBytes.empty()) {
    if (packing.tableBaseAddress < kernelStart ||
        packing.tableBaseAddress + packing.tableBytes.size() > kernelEnd) {
      result.error = "central table is outside the composed kernel page";
      return result;
    }
    std::copy(packing.tableBytes.begin(), packing.tableBytes.end(),
              result.image.begin() +
                  (packing.tableBaseAddress - kernelStart));
  }

  std::map<std::string, unsigned int> importStubs;
  for (std::size_t i = 0; i < packing.dispatches.size(); i++) {
    importStubs[packing.dispatches[i].exportName] =
        packing.dispatches[i].stubAddress;
  }
  for (std::size_t i = 0; i < packing.imports.size(); i++) {
    importStubs[packing.imports[i].targetExport] =
        packing.imports[i].stubAddress;
  }

  for (std::size_t i = 0; i < packing.placements.size(); i++) {
    const KernelModulePlacement& placement = packing.placements[i];
    if (!placement.resident) continue;

    const KernelModule* module = nullptr;
    for (std::size_t j = 0; j < library.modules().size(); j++) {
      if (library.modules()[j].id == placement.moduleId) {
        module = &library.modules()[j];
        break;
      }
    }
    if (!module) {
      result.error = "module '" + placement.moduleId + "' is missing";
      return result;
    }

    if (placement.address < kernelStart ||
        placement.address + module->size() > kernelEnd) {
      result.error = "module '" + module->id +
                     "' is outside the composed kernel page";
      return result;
    }

    std::vector<unsigned char> bytes = module->bytes;
    if (module->assemblyBase != placement.address) {
      std::string error;
      if (!applyRelocations(bytes, module->assemblyBase, placement.address,
                            module->relocations, error)) {
        result.error = "module '" + module->id + "': " + error;
        return result;
      }
    }
    if (!module->imports.empty()) {
      std::string error;
      if (!applyModuleImports(bytes, module->imports, importStubs, error)) {
        result.error = "module '" + module->id + "': " + error;
        return result;
      }
    }

    std::copy(bytes.begin(), bytes.end(),
              result.image.begin() + (placement.address - kernelStart));
  }

  result.ok = true;
  return result;
}
