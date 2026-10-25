/***
 * @file dispatch_fixup_resolver.h
 * @brief Resolve deferred kernel dispatch fixups
 */

#ifndef DISPATCH_FIXUP_RESOLVER_H_INCLUDED
#define DISPATCH_FIXUP_RESOLVER_H_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "dispatch_fixup.h"

bool applyDispatchFixups(
    const std::vector<DispatchFixup>& fixups,
    const std::map<unsigned int, unsigned int>& stubAddresses,
    std::vector<unsigned char>& code, std::string& error);

#endif  // DISPATCH_FIXUP_RESOLVER_H_INCLUDED
