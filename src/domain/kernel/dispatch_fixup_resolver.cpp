/***
 * @file dispatch_fixup_resolver.cpp
 * @brief Resolve deferred kernel dispatch fixups
 */

#include "dispatch_fixup_resolver.h"

bool applyDispatchFixups(
    const std::vector<DispatchFixup>& fixups,
    const std::map<unsigned int, unsigned int>& stubAddresses,
    std::vector<unsigned char>& code, std::string& error) {
  for (std::size_t i = 0; i < fixups.size(); i++) {
    const DispatchFixup& fixup = fixups[i];
    std::map<unsigned int, unsigned int>::const_iterator entry =
        stubAddresses.find(fixup.dispatchIndex);
    if (entry == stubAddresses.end()) {
      error = "no stub address for dispatch index " +
              std::to_string(fixup.dispatchIndex);
      return false;
    }
    if (fixup.address + 1 >= code.size()) {
      error = "dispatch fixup is outside the code buffer";
      return false;
    }
    code[fixup.address] = static_cast<unsigned char>(entry->second & 0xFF);
    code[fixup.address + 1] =
        static_cast<unsigned char>((entry->second >> 8) & 0xFF);
  }
  return true;
}
