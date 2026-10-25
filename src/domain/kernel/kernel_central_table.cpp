/***
 * @file kernel_central_table.cpp
 * @brief Runtime central jump table stub generation
 */

#include "kernel_central_table.h"

#include <algorithm>
#include <set>

namespace {

bool byDispatchIndex(const KernelDispatchStub& left,
                     const KernelDispatchStub& right) {
  return left.dispatchIndex < right.dispatchIndex;
}

}  // namespace

std::vector<unsigned char> KernelCentralTable::residentStub(
    unsigned int target) {
  std::vector<unsigned char> bytes;
  bytes.push_back(0xC3);
  bytes.push_back(static_cast<unsigned char>(target & 0xFF));
  bytes.push_back(static_cast<unsigned char>((target >> 8) & 0xFF));
  return bytes;
}

std::vector<unsigned char> KernelCentralTable::pagedStub(
    unsigned int dispatcher, unsigned int segment, unsigned int target) {
  std::vector<unsigned char> bytes;
  bytes.push_back(0xCD);
  bytes.push_back(static_cast<unsigned char>(dispatcher & 0xFF));
  bytes.push_back(static_cast<unsigned char>((dispatcher >> 8) & 0xFF));
  bytes.push_back(static_cast<unsigned char>(segment & 0xFF));
  bytes.push_back(static_cast<unsigned char>(target & 0xFF));
  bytes.push_back(static_cast<unsigned char>((target >> 8) & 0xFF));
  return bytes;
}

bool KernelCentralTable::build(
    const std::vector<KernelDispatchStub>& stubs,
    unsigned int dispatcherAddress, unsigned int baseAddress,
    std::vector<unsigned char>& bytes,
    std::map<unsigned int, unsigned int>& stubAddresses, std::string& error) {
  bytes.clear();
  stubAddresses.clear();

  if (dispatcherAddress < 0x4000u || dispatcherAddress >= 0x8000u) {
    error = "dispatcher address is outside the fixed kernel page";
    return false;
  }

  std::vector<KernelDispatchStub> ordered = stubs;
  std::sort(ordered.begin(), ordered.end(), byDispatchIndex);

  std::set<unsigned int> seen;
  unsigned int cursor = baseAddress;

  for (std::size_t i = 0; i < ordered.size(); i++) {
    const KernelDispatchStub& stub = ordered[i];

    if (!seen.insert(stub.dispatchIndex).second) {
      error = "duplicate dispatch index in central table";
      return false;
    }

    if (stub.address < 0x4000u || stub.address > 0xFFFFu) {
      error = "dispatch target is outside the address space";
      return false;
    }

    std::vector<unsigned char> entry;
    if (stub.paged) {
      entry = pagedStub(dispatcherAddress, stub.segment, stub.address);
    } else {
      entry = residentStub(stub.address);
    }

    if (cursor < 0x4000u || cursor + entry.size() > 0x8000u) {
      error = "central table exceeds the fixed kernel page";
      return false;
    }

    stubAddresses[stub.dispatchIndex] = cursor;
    bytes.insert(bytes.end(), entry.begin(), entry.end());
    cursor += static_cast<unsigned int>(entry.size());
  }

  return true;
}
