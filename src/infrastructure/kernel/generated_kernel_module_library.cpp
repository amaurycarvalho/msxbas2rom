/***
 * @file generated_kernel_module_library.cpp
 * @brief Generated kernel module library adapter implementation
 */

#include "generated_kernel_module_library.h"

#include "kernel_dispatch_generated.h"
#include "kernel_manifest_generated.h"
#include "kernel_relocatable_generated.h"

static void applyRelocatable(
    KernelModule& module,
    const GeneratedRelocatableModuleDefinition& definition) {
  module.owner = definition.owner;
  module.group = definition.group;
  module.classification = definition.classification;
  module.policy = static_cast<KernelPlacementPolicy>(definition.policy);
  module.forceOverflow = definition.force_overflow != 0;
  module.bytes.assign(definition.bytes, definition.bytes + definition.size);
  module.assemblyBase = 0;
  module.fixedAddress = 0;
  module.hasFixedAddress = false;

  module.exports.clear();
  for (unsigned int k = 0; k < definition.exports_count; k++) {
    KernelExport entry;
    entry.name = definition.exports[k].name;
    entry.offset = definition.exports[k].offset;
    module.exports.push_back(entry);
  }

  module.imports.clear();
  for (unsigned int k = 0; k < definition.imports_count; k++) {
    KernelImport entry;
    entry.target = definition.imports[k].target;
    entry.offset = definition.imports[k].offset;
    entry.kind =
        static_cast<KernelImportKind>(definition.imports[k].kind);
    module.imports.push_back(entry);
  }

  module.relocations.clear();
  for (unsigned int k = 0; k < definition.relocations_count; k++) {
    KernelRelocation relocation;
    relocation.offset = definition.relocations[k];
    module.relocations.push_back(relocation);
  }

  module.dependencies.clear();
  for (unsigned int k = 0; k < definition.dependencies_count; k++) {
    module.dependencies.push_back(definition.dependencies[k]);
  }

  module.patchPoints.clear();
}

GeneratedKernelModuleLibrary::GeneratedKernelModuleLibrary() {
  for (unsigned int i = 0; i < KERNEL_MANIFEST_MODULE_COUNT; i++) {
    const GeneratedKernelModuleDefinition& definition =
        kernel_manifest_modules[i];

    KernelModule module;
    module.id = definition.id;
    module.name = definition.name;
    module.owner = definition.owner;
    module.group = definition.group;
    module.classification = definition.classification;
    module.bytes.assign(definition.bytes,
                        definition.bytes + definition.size);
    module.assemblyBase = definition.assembly_base;
    module.fixedAddress = definition.fixed_address;
    module.hasFixedAddress = definition.has_fixed_address != 0;
    module.policy = static_cast<KernelPlacementPolicy>(definition.policy);

    for (unsigned int j = 0; j < definition.exports_count; j++) {
      KernelExport entry;
      entry.name = definition.exports[j].name;
      entry.offset = definition.exports[j].offset;
      module.exports.push_back(entry);
    }

    for (unsigned int j = 0; j < definition.imports_count; j++) {
      KernelImport entry;
      entry.target = definition.imports[j].target;
      entry.offset = definition.imports[j].offset;
      entry.kind = static_cast<KernelImportKind>(
          definition.imports[j].kind);
      module.imports.push_back(entry);
    }

    for (unsigned int j = 0; j < definition.relocations_count; j++) {
      KernelRelocation relocation;
      relocation.offset = definition.relocations[j];
      module.relocations.push_back(relocation);
    }

    for (unsigned int j = 0; j < definition.dependencies_count; j++) {
      module.dependencies.push_back(definition.dependencies[j]);
    }

    for (unsigned int j = 0; j < definition.patch_points_count; j++) {
      module.patchPoints.push_back(definition.patch_points[j]);
    }

    moduleList.push_back(module);
  }

  for (unsigned int i = 0; i < KERNEL_MANIFEST_FIXED_REGION_COUNT; i++) {
    const GeneratedKernelFixedRegionDefinition& definition =
        kernel_manifest_fixed_regions[i];
    KernelFixedRegion region;
    region.id = definition.id;
    region.start = definition.start;
    region.end = definition.end;
    region.owner = definition.owner;
    regionList.push_back(region);
  }

  for (unsigned int i = 0; i < KERNEL_DISPATCH_COUNT; i++) {
    dispatchMap[i] = kernel_dispatch_names[i];
  }

  for (unsigned int i = 0; i < KERNEL_RELOCATABLE_MODULE_COUNT; i++) {
    const GeneratedRelocatableModuleDefinition& definition =
        kernel_relocatable_modules[i];
    bool replaced = false;
    for (std::size_t j = 0; j < moduleList.size(); j++) {
      if (moduleList[j].id != definition.id) continue;
      applyRelocatable(moduleList[j], definition);
      replaced = true;
      break;
    }
    if (!replaced) {
      KernelModule module;
      module.id = definition.id;
      module.name = definition.id;
      applyRelocatable(module, definition);
      moduleList.push_back(module);
    }
  }
}

const std::vector<KernelModule>& GeneratedKernelModuleLibrary::modules() const {
  return moduleList;
}

const std::vector<KernelFixedRegion>&
GeneratedKernelModuleLibrary::fixedRegions() const {
  return regionList;
}

const std::map<unsigned int, std::string>&
GeneratedKernelModuleLibrary::dispatchExports() const {
  return dispatchMap;
}
