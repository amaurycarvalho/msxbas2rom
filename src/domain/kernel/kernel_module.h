/***
 * @file kernel_module.h
 * @brief Kernel module manifest and relocation model
 */

#ifndef KERNEL_MODULE_H_INCLUDED
#define KERNEL_MODULE_H_INCLUDED

#include <map>
#include <string>
#include <vector>

enum class KernelPlacementPolicy { Fixed, FlexHigh, FlexLow, Overflow };

const char* kernelPlacementPolicyName(KernelPlacementPolicy policy);
bool parseKernelPlacementPolicy(const std::string& text,
                                KernelPlacementPolicy& policy);

struct KernelExport {
  std::string name;
  unsigned int offset;
};

struct KernelRelocation {
  unsigned int offset;
};

enum class KernelImportKind {
  Call = 0,
  Jump = 1,
  Conditional = 2,
  Address = 3
};

struct KernelImport {
  std::string target;
  unsigned int offset;
  KernelImportKind kind;
};

struct KernelFixedRegion {
  std::string id;
  unsigned int start;
  unsigned int end;
  std::string owner;
};

class KernelModule {
 public:
  std::string id;
  std::string name;
  std::string owner;
  std::string group;
  std::string classification;
  std::vector<unsigned char> bytes;
  unsigned int assemblyBase;
  KernelPlacementPolicy policy;
  unsigned int fixedAddress;
  bool hasFixedAddress;
  bool forceOverflow;
  std::vector<KernelExport> exports;
  std::vector<KernelImport> imports;
  std::vector<KernelRelocation> relocations;
  std::vector<std::string> dependencies;
  std::vector<unsigned int> patchPoints;

  KernelModule();

  unsigned int size() const;
  unsigned int end() const;
  const KernelExport* findExport(const std::string& name) const;
};

class KernelModuleManifest {
 public:
  std::vector<KernelModule> modules;
  std::vector<KernelFixedRegion> fixedRegions;

  const KernelModule* findModule(const std::string& id) const;
  const KernelModule* findModuleForExport(const std::string& name) const;
  bool validate(std::string& error) const;
  bool validateAcyclic(std::string& error) const;
};

struct KernelRelocationDiff {
  std::vector<unsigned int> offsets;
  std::vector<unsigned int> values;
};

/***
 * @brief Detect 16-bit words that differ between two assemblies of the same
 * module assembled at different bases.
 */
bool diffRelocations(const std::vector<unsigned char>& baseA,
                     const std::vector<unsigned char>& baseB, int baseDelta,
                     KernelRelocationDiff& diff, std::string& error);

/***
 * @brief Apply relocation records to a module image.
 */
bool applyRelocations(std::vector<unsigned char>& bytes,
                      unsigned int assemblyBase, unsigned int finalBase,
                      const std::vector<KernelRelocation>& relocations,
                      std::string& error);

/***
 * @brief Apply cross-module import fixups to a module image.
 */
bool applyModuleImports(
    std::vector<unsigned char>& bytes,
    const std::vector<KernelImport>& imports,
    const std::map<std::string, unsigned int>& stubAddresses,
    std::string& error);

#endif  // KERNEL_MODULE_H_INCLUDED
